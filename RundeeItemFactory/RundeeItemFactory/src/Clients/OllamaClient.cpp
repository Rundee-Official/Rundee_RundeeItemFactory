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

std::string OllamaClient::RunSimple(const std::string& modelName, const std::string& prompt)
{
    std::string flatPrompt = StringUtils::NormalizeWhitespace(prompt);
    std::string escapedPrompt = StringUtils::EscapeForCmd(flatPrompt);
    std::string command = "ollama run " + modelName +
        " \"" + escapedPrompt + "\"";

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
            }
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


