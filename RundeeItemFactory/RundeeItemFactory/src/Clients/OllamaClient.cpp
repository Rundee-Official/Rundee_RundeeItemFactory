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
#include "Utils/StringUtils.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace
{
    // Build JSON request body for Ollama API
    std::string BuildJsonRequest(const std::string& modelName, const std::string& prompt)
    {
        std::ostringstream json;
        json << "{\"model\":\"" << modelName << "\",\"prompt\":\"";
        
        // Escape JSON special characters in prompt
        for (char c : prompt)
        {
            if (c == '"')
                json << "\\\"";
            else if (c == '\\')
                json << "\\\\";
            else if (c == '\n')
                json << "\\n";
            else if (c == '\r')
                json << "\\r";
            else if (c == '\t')
                json << "\\t";
            else
                json << c;
        }
        
        json << "\",\"stream\":false}";
        return json.str();
    }

    // Extract response text from Ollama JSON response
    // Ollama API can return multiple JSON objects (one per line) when stream: false
    // Each line: {"response": "text_chunk", "done": false}
    // Last line: {"response": "text_chunk", "done": true}
    std::string ExtractResponseFromJson(const std::string& jsonResponse)
    {
        std::string result;
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

std::string OllamaClient::RunSimple(const std::string& modelName, const std::string& prompt)
{
    std::cout << "[OllamaClient] Calling Ollama HTTP API: model=" << modelName << "\n";
    
    // Build JSON request
    std::string jsonRequest = BuildJsonRequest(modelName, prompt);
    
    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(L"RundeeItemFactory/1.0",
                                     WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME,
                                     WINHTTP_NO_PROXY_BYPASS, 0);
    
    if (!hSession)
    {
        std::cerr << "[OllamaClient] WinHttpOpen failed. Error: " << GetLastError() << "\n";
        return {};
    }
    
    // Connect to Ollama server
    HINTERNET hConnect = WinHttpConnect(hSession, L"localhost", 11434, 0);
    
    if (!hConnect)
    {
        std::cerr << "[OllamaClient] WinHttpConnect failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hSession);
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
        std::cerr << "[OllamaClient] WinHttpOpenRequest failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }
    
    // Set headers
    std::wstring headers = L"Content-Type: application/json\r\n";
    if (!WinHttpAddRequestHeaders(hRequest, headers.c_str(), -1, WINHTTP_ADDREQ_FLAG_ADD))
    {
        std::cerr << "[OllamaClient] WinHttpAddRequestHeaders failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }
    
    // Set timeout (120 seconds)
    DWORD timeout = 120000; // 120 seconds in milliseconds
    WinHttpSetTimeouts(hRequest, timeout, timeout, timeout, timeout);
    
    // Send request
    if (!WinHttpSendRequest(hRequest,
                            WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                            (LPVOID)jsonRequest.c_str(),
                            (DWORD)jsonRequest.length(),
                            (DWORD)jsonRequest.length(), 0))
    {
        std::cerr << "[OllamaClient] WinHttpSendRequest failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }
    
    // Receive response
    if (!WinHttpReceiveResponse(hRequest, NULL))
    {
        std::cerr << "[OllamaClient] WinHttpReceiveResponse failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
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
        std::cerr << "[OllamaClient] WinHttpQueryHeaders failed. Error: " << GetLastError() << "\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }
    
    if (statusCode != 200)
    {
        std::cerr << "[OllamaClient] HTTP request failed with status code: " << statusCode << "\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return {};
    }
    
    // Read response data
    std::string response;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    char buffer[8192];
    
    do
    {
        if (!WinHttpQueryDataAvailable(hRequest, &bytesAvailable))
        {
            std::cerr << "[OllamaClient] WinHttpQueryDataAvailable failed. Error: " << GetLastError() << "\n";
            break;
        }
        
        if (bytesAvailable == 0)
            break;
        
        if (!WinHttpReadData(hRequest, buffer, sizeof(buffer) - 1, &bytesRead))
        {
            std::cerr << "[OllamaClient] WinHttpReadData failed. Error: " << GetLastError() << "\n";
            break;
        }
        
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            response.append(buffer, bytesRead);
        }
    } while (bytesRead > 0);
    
    // Clean up
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    
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
    
    return trimmed;
}

std::string OllamaClient::RunWithRetry(const std::string& modelName, 
                                       const std::string& prompt,
                                       int maxRetries,
                                       int timeoutSeconds)
{
    std::string result;
    
    for (int attempt = 1; attempt <= maxRetries; ++attempt)
    {
        std::cout << "[OllamaClient] Attempt " << attempt << " of " << maxRetries << "\n";
        
        result = RunSimple(modelName, prompt);
        
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
        
        // If this is not the last attempt, wait before retrying
        if (attempt < maxRetries)
        {
            int waitSeconds = 2 * attempt; // Exponential backoff: 2, 4, 6 seconds
            std::cout << "[OllamaClient] Response invalid or empty. Retrying in " << waitSeconds << " seconds...\n";
            std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
        }
        else
        {
            std::cerr << "[OllamaClient] All " << maxRetries << " attempts failed.\n";
        }
    }
    
    return {}; // Return empty string if all attempts failed
}


