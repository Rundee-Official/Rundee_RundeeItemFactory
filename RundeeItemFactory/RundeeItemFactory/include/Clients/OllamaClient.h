// ===============================
// Project Name: RundeeItemFactory
// File Name: OllamaClient.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Simple helper for calling Ollama from C++ (Windows, using HTTP API).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>
#include "Helpers/AppConfig.h"

class OllamaClient
{
public:
    // Simple helper function:
    // Calls Ollama HTTP API with the given model name and prompt,
    // Returns the JSON response as a string.
    // Returns empty string on error.
    static std::string RunSimple(const std::string& modelName, 
                                 const std::string& prompt,
                                 const OllamaSettings& settings);

    // Run with retry logic:
    // Attempts to call LLM with automatic retry on failure.
    // maxRetries: Maximum number of retry attempts (default: 3)
    // timeoutSeconds: Timeout per attempt in seconds (default: 120, 0 = no timeout)
    // Returns the response string, or empty string if all attempts failed.
    static std::string RunWithRetry(const std::string& modelName, 
                                    const std::string& prompt,
                                    int maxRetries = 3,
                                    int timeoutSeconds = 120);
};


