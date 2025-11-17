// ===============================
// Project Name: RundeeItemFactory
// File Name: OllamaClient.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Simple helper for calling Ollama from C++ (Windows).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Clients/OllamaClient.h"
#include "Utils/StringUtils.h"
#include <iostream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <fstream>
#include <windows.h>

namespace
{
    std::string WritePromptToTempFile(const std::string& prompt)
    {
        char tempPathBuffer[MAX_PATH];
        DWORD tempPathLen = GetTempPathA(MAX_PATH, tempPathBuffer);
        if (tempPathLen == 0 || tempPathLen > MAX_PATH)
        {
            std::cerr << "[OllamaClient] GetTempPath failed.\n";
            return {};
        }

        char tempFileName[MAX_PATH];
        if (GetTempFileNameA(tempPathBuffer, "OLP", 0, tempFileName) == 0)
        {
            std::cerr << "[OllamaClient] GetTempFileName failed.\n";
            return {};
        }

        std::ofstream ofs(tempFileName, std::ios::binary);
        if (!ofs)
        {
            std::cerr << "[OllamaClient] Failed to open temp prompt file for writing: "
                << tempFileName << "\n";
            return {};
        }

        ofs << prompt;
        ofs.close();
        return std::string(tempFileName);
    }
}

std::string OllamaClient::RunSimple(const std::string& modelName, const std::string& prompt)
{
    std::string flatPrompt = StringUtils::NormalizeWhitespace(prompt);
    std::string tempPromptPath = WritePromptToTempFile(flatPrompt);
    if (tempPromptPath.empty())
    {
        return {};
    }

    std::string command =
        "set \"OLLAMA_NO_ANSI=1\" && set \"OLLAMA_NO_COLOR=1\" && "
        "type \"" + tempPromptPath + "\" | ollama run " + modelName + " 2>&1";

    std::cout << "[OllamaClient] Running command: " << command << "\n";

    std::string result;

    // Execute command with _popen and read stdout
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe)
    {
        std::cerr << "[OllamaClient] Failed to open pipe.\n";
        return {};
    }

    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }

    int rc = _pclose(pipe);
    if (rc != 0)
    {
        std::cerr << "[OllamaClient] Command returned code: " << rc << "\n";
    }

    result = StringUtils::StripAnsiEscapeCodes(result);
    result = StringUtils::RemoveControlCharacters(result);

    size_t firstBracket = result.find_first_of("[{");
    if (firstBracket != std::string::npos)
    {
        if (firstBracket > 0)
        {
            std::cout << "[OllamaClient] Trimmed " << firstBracket
                << " leading chars before JSON start.\n";
        }
        result = result.substr(firstBracket);
    }

    size_t lastBracket = result.find_last_of("]}");
    if (lastBracket != std::string::npos && lastBracket + 1 < result.size())
    {
        size_t trimmedCount = result.size() - (lastBracket + 1);
        if (trimmedCount > 0)
        {
            std::cout << "[OllamaClient] Trimmed " << trimmedCount
                << " trailing chars after JSON end.\n";
            result = result.substr(0, lastBracket + 1);
        }
    }

    if (!tempPromptPath.empty())
    {
        if (!DeleteFileA(tempPromptPath.c_str()))
        {
            std::cerr << "[OllamaClient] Warning: Failed to delete temp prompt file: "
                << tempPromptPath << " (error " << GetLastError() << ")\n";
        }
    }

    return result;
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


