// ===============================
// Project Name: RundeeItemFactory
// File Name: OllamaClient.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Simple helper for calling Ollama from C++ (Windows, using HTTP API).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Clients/OllamaClient.h"
#include "Helpers/AppConfig.h"
#include "Utils/StringUtils.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace
{
    /**
     * @brief Safely close a WinHTTP handle, checking for NULL first
     * @param handle WinHTTP handle to close (can be NULL)
     * 
     * Helper function that safely closes WinHTTP handles without crashing
     * if the handle is NULL. Used for cleanup in error paths.
     */
    inline void SafeCloseHandle(HINTERNET handle)
    {
        if (handle != NULL)
        {
            WinHttpCloseHandle(handle);
        }
    }

    /**
     * @brief Build JSON request body for Ollama API
     * @param modelName Name of the LLM model (e.g., "llama3")
     * @param prompt Prompt text to send to the LLM
     * @return JSON string in format: {"model":"name","prompt":"text","stream":false}
     * 
     * Escapes JSON special characters in the prompt text to ensure valid JSON.
     * Pre-allocates string memory for performance optimization.
     */
    std::string BuildJsonRequest(const std::string& modelName, const std::string& prompt)
    {
        // Pre-allocate string with estimated size (model name + prompt length + overhead)
        std::string json;
        json.reserve(modelName.length() + prompt.length() * 2 + 50); // Reserve extra space for escaped chars
        
        json = "{\"model\":\"";
        json += modelName;
        json += "\",\"prompt\":\"";
        
        // Escape JSON special characters in prompt
        for (char c : prompt)
        {
            if (c == '"')
                json += "\\\"";
            else if (c == '\\')
                json += "\\\\";
            else if (c == '\n')
                json += "\\n";
            else if (c == '\r')
                json += "\\r";
            else if (c == '\t')
                json += "\\t";
            else
                json += c;
        }
        
        json += "\",\"stream\":false}";
        return json;
    }

    /**
     * @brief Extract response text from Ollama JSON response
     * @param jsonResponse Raw JSON response string from Ollama API
     * @return Extracted response text with all chunks concatenated
     * 
     * Ollama API can return multiple JSON objects (one per line) when stream: false.
     * Each line format: {"response": "text_chunk", "done": false}
     * Last line format: {"response": "text_chunk", "done": true}
     * 
     * This function:
     * - Parses each line as a separate JSON object
     * - Extracts the "response" field from each line
     * - Unescapes JSON escape sequences
     * - Concatenates all chunks into a single string
     * 
     * Pre-allocates result string for performance optimization.
     */
    std::string ExtractResponseFromJson(const std::string& jsonResponse)
    {
        std::string result;
        result.reserve(jsonResponse.length() / 2); // Pre-allocate estimated size
        std::istringstream stream(jsonResponse);
        std::string line;
        
        // Process each line (each line is a JSON object)
        while (std::getline(stream, line))
        {
            // Skip empty lines
            if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos)
                continue;
            
            // Find "response" field in this line
            size_t responseStart = line.find("\"response\":\"");
            if (responseStart == std::string::npos)
            {
                // Try without quotes (in case of unquoted value, though unlikely)
                responseStart = line.find("\"response\":");
                if (responseStart == std::string::npos)
                {
                    continue; // Skip this line if no response field
                }
                responseStart += 11; // Skip "response":
                
                // Skip whitespace
                while (responseStart < line.length() && 
                       (line[responseStart] == ' ' || line[responseStart] == '\t'))
                {
                    responseStart++;
                }
                
                // Check if it's a quoted string
                if (responseStart < line.length() && line[responseStart] == '"')
                {
                    responseStart++; // Skip opening quote
                }
            }
            else
            {
                responseStart += 12; // Skip "response":"
            }
            
            // Find the end of the response string value
            size_t responseEnd = responseStart;
            bool escaped = false;
            
            while (responseEnd < line.length())
            {
                if (escaped)
                {
                    escaped = false;
                    responseEnd++;
                    continue;
                }
                
                if (line[responseEnd] == '\\')
                {
                    escaped = true;
                    responseEnd++;
                    continue;
                }
                
                if (line[responseEnd] == '"')
                {
                    break;
                }
                
                responseEnd++;
            }
            
            if (responseEnd > responseStart)
            {
                std::string extracted = line.substr(responseStart, responseEnd - responseStart);
                
                // Unescape JSON escape sequences and append to result
                for (size_t i = 0; i < extracted.length(); ++i)
                {
                    if (extracted[i] == '\\' && i + 1 < extracted.length())
                    {
                        switch (extracted[i + 1])
                        {
                        case 'n':
                            result += '\n';
                            i++;
                            break;
                        case 'r':
                            result += '\r';
                            i++;
                            break;
                        case 't':
                            result += '\t';
                            i++;
                            break;
                        case '\\':
                            result += '\\';
                            i++;
                            break;
                        case '"':
                            result += '"';
                            i++;
                            break;
                        default:
                            result += extracted[i];
                            break;
                        }
                    }
                    else
                    {
                        result += extracted[i];
                    }
                }
            }
        }
        
        // If we couldn't extract anything, return the original response
        // (might be that the response is already the final text)
        if (result.empty())
        {
            return jsonResponse;
        }
        
        return result;
    }
}

std::string OllamaClient::RunSimple(const std::string& modelName, 
                                    const std::string& prompt,
                                    const OllamaSettings& settings)
{
    const std::string host = settings.host.empty() ? "localhost" : settings.host;
    const INTERNET_PORT port = static_cast<INTERNET_PORT>(settings.port > 0 ? settings.port : 11434);
    const DWORD requestTimeoutMs = static_cast<DWORD>((settings.requestTimeoutSeconds > 0 ? settings.requestTimeoutSeconds : 120) * 1000);
    const DWORD connectTimeout = static_cast<DWORD>(settings.connectTimeoutMs > 0 ? settings.connectTimeoutMs : 5000);
    const DWORD sendTimeout = static_cast<DWORD>(settings.sendTimeoutMs > 0 ? settings.sendTimeoutMs : requestTimeoutMs);
    const DWORD receiveTimeout = static_cast<DWORD>(settings.receiveTimeoutMs > 0 ? settings.receiveTimeoutMs : requestTimeoutMs);

    std::cout << "[OllamaClient] Calling Ollama HTTP API (model=" << modelName
        << ", host=" << host << ", port=" << port << ")\n";
    
    // Build JSON request
    std::string jsonRequest = BuildJsonRequest(modelName, prompt);
    
    // Convert host to wide string
    int wideLen = MultiByteToWideChar(CP_UTF8, 0, host.c_str(), -1, nullptr, 0);
    std::wstring hostW;
    hostW.resize(wideLen);
    MultiByteToWideChar(CP_UTF8, 0, host.c_str(), -1, &hostW[0], wideLen);
    if (!hostW.empty() && hostW.back() == L'\0')
    {
        hostW.pop_back();
    }

    auto requestStart = std::chrono::steady_clock::now();

    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(L"RundeeItemFactory/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    
    if (!hSession)
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpOpen failed. Error code: " << error 
                  << ". Check if WinHTTP is properly installed.\n";
        return {};
    }
    
    // Connect to Ollama server
    HINTERNET hConnect = WinHttpConnect(hSession, hostW.c_str(), port, 0);
    
    if (!hConnect)
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpConnect failed. Error code: " << error 
                  << " (host=" << host << ", port=" << port << ")\n";
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Open request
    HINTERNET hRequest = WinHttpOpenRequest(hConnect,
                                            L"POST",
                                            L"/api/generate",
                                            NULL,
                                            WINHTTP_NO_REFERER,
                                            WINHTTP_DEFAULT_ACCEPT_TYPES,
                                            0);
    
    if (!hRequest)
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpOpenRequest failed. Error code: " << error << "\n";
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Set headers
    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD))
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpAddRequestHeaders failed. Error code: " << error << "\n";
        SafeCloseHandle(hRequest);
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Set timeout values
    WinHttpSetTimeouts(hRequest, connectTimeout, connectTimeout, sendTimeout, receiveTimeout);
    
    // Send request
    if (!WinHttpSendRequest(hRequest,
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            (LPVOID)jsonRequest.c_str(),
                            (DWORD)jsonRequest.length(),
                            (DWORD)jsonRequest.length(), 0))
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpSendRequest failed. Error code: " << error 
                  << " (request size: " << jsonRequest.length() << " bytes)\n";
        SafeCloseHandle(hRequest);
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Receive response
    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpReceiveResponse failed. Error code: " << error << "\n";
        SafeCloseHandle(hRequest);
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Check status code
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    if (!WinHttpQueryHeaders(hRequest,
                              WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                              WINHTTP_HEADER_NAME_BY_INDEX,
                              &statusCode,
                              &statusCodeSize,
                              WINHTTP_NO_HEADER_INDEX))
    {
        DWORD error = GetLastError();
        std::cerr << "[OllamaClient] WinHttpQueryHeaders failed. Error code: " << error << "\n";
        SafeCloseHandle(hRequest);
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    if (statusCode != 200)
    {
        std::cerr << "[OllamaClient] HTTP request failed with status code: " << statusCode 
                  << " (expected 200 OK). Server may be unavailable or request invalid.\n";
        SafeCloseHandle(hRequest);
        SafeCloseHandle(hConnect);
        SafeCloseHandle(hSession);
        return {};
    }
    
    // Read response data
    std::string response;
    response.reserve(32768); // Pre-allocate 32KB for typical responses
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    char buffer[8192];
    
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable))
        {
            DWORD error = GetLastError();
            // Only log error if it's not a normal completion (ERROR_NO_MORE_FILES = 18)
            if (error != ERROR_NO_MORE_FILES)
            {
                std::cerr << "[OllamaClient] WinHttpQueryDataAvailable failed. Error code: " << error << "\n";
            }
            break;
        }
        
        if (bytesAvailable == 0)
            break;
        
        if (!WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead))
        {
            DWORD error = GetLastError();
            // Only log error if it's not a normal completion
            if (error != ERROR_NO_MORE_FILES)
            {
                std::cerr << "[OllamaClient] WinHttpReadData failed. Error code: " << error << "\n";
            }
            break;
        }
        
        if (bytesRead > 0)
        {
            response.append(buffer, bytesRead);
        }
    } while (bytesRead > 0);
    
    // Clean up handles (always executed, even if errors occurred during reading)
    SafeCloseHandle(hRequest);
    SafeCloseHandle(hConnect);
    SafeCloseHandle(hSession);
    
    if (response.empty())
    {
        std::cerr << "[OllamaClient] Received empty response.\n";
        return {};
    }
    
    // Extract the actual response text from Ollama JSON
    std::string extractedResponse = ExtractResponseFromJson(response);
    
    // Clean up the response (remove any leading/trailing whitespace)
    std::string trimmed = extractedResponse;
    size_t start = trimmed.find_first_not_of(" \t\r\n");
    if (start != std::string::npos)
    {
        trimmed = trimmed.substr(start);
    }
    
    size_t end = trimmed.find_last_not_of(" \t\r\n");
    if (end != std::string::npos)
    {
        trimmed = trimmed.substr(0, end + 1);
    }
    
    // Try to find JSON array/object in the response
    size_t firstBracket = trimmed.find_first_of("[{");
    if (firstBracket != std::string::npos)
    {
        if (firstBracket > 0)
        {
            std::cout << "[OllamaClient] Trimmed " << firstBracket
                << " leading chars before JSON start.\n";
        }
        trimmed = trimmed.substr(firstBracket);
    }
    
    size_t lastBracket = trimmed.find_last_of("]}");
    if (lastBracket != std::string::npos && lastBracket + 1 < trimmed.size())
    {
        size_t trimmedCount = trimmed.size() - (lastBracket + 1);
        if (trimmedCount > 0)
        {
            std::cout << "[OllamaClient] Trimmed " << trimmedCount
                << " trailing chars after JSON end.\n";
            trimmed = trimmed.substr(0, lastBracket + 1);
        }
    }
    
    auto requestEnd = std::chrono::steady_clock::now();
    double durationSeconds = std::chrono::duration<double>(requestEnd - requestStart).count();
    std::cout << "[OllamaClient] HTTP call succeeded in " << std::fixed << std::setprecision(2)
        << durationSeconds << "s (" << host << ":" << port << ")\n";

    return trimmed;
}

std::string OllamaClient::RunWithRetry(const std::string& modelName, 
                                       const std::string& prompt,
                                       int maxRetries,
                                       int timeoutSeconds)
{
    const OllamaSettings& config = AppConfig::GetOllamaSettings();
    OllamaSettings effective = config;

    if (timeoutSeconds > 0)
    {
        effective.requestTimeoutSeconds = timeoutSeconds;
    }
    if (maxRetries > 0)
    {
        effective.maxRetries = maxRetries;
    }

    if (effective.maxRetries <= 0)
        effective.maxRetries = 3;
    if (effective.requestTimeoutSeconds <= 0)
        effective.requestTimeoutSeconds = 120;

    std::string result;
    
    for (int attempt = 1; attempt <= effective.maxRetries; ++attempt)
    {
        std::cout << "[OllamaClient] Attempt " << attempt << " of " << effective.maxRetries << "\n";
        
        result = RunSimple(modelName, prompt, effective);
        
        // Check if result is valid (not empty)
        if (!result.empty())
        {
            // Check if result looks like valid JSON (starts with [ or {)
            std::string trimmed = result;
            size_t start = trimmed.find_first_not_of(" \t\r\n");
            if (start != std::string::npos)
            {
                trimmed = trimmed.substr(start);
                if (trimmed[0] == '[' || trimmed[0] == '{')
                {
                    std::cout << "[OllamaClient] Successfully received response on attempt " << attempt << "\n";
                    return result;
                }
                else
                {
                    std::string preview = trimmed.substr(0, 200);
                    std::cout << "[OllamaClient] Received non-JSON response (preview): " << preview << "\n";
                }
            }
            else
            {
                std::cout << "[OllamaClient] Response contained only whitespace.\n";
            }
        }
        else
        {
            std::cout << "[OllamaClient] Response was empty.\n";
        }
        
        // If this is not the last attempt, wait before retrying with exponential backoff
        if (attempt < effective.maxRetries)
        {
            // True exponential backoff: baseDelay * (2^(attempt-1))
            // attempt 1: 1 * 2^0 = 1 second
            // attempt 2: 1 * 2^1 = 2 seconds  
            // attempt 3: 1 * 2^2 = 4 seconds
            // attempt 4: 1 * 2^3 = 8 seconds
            // Maximum cap at 60 seconds to avoid excessive delays
            const int baseDelay = 1;
            int waitSeconds = baseDelay * (1 << (attempt - 1)); // 2^(attempt-1)
            if (waitSeconds > 60)
                waitSeconds = 60; // Cap at 60 seconds
            std::cout << "[OllamaClient] Response invalid or empty. Retrying in " << waitSeconds << " seconds... (attempt " << attempt << "/" << effective.maxRetries << ")\n";
            std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
        }
        else
        {
            std::cerr << "[OllamaClient] All " << maxRetries << " attempts failed.\n";
        }
    }
    
    return {}; // Return empty string if all attempts failed
}
