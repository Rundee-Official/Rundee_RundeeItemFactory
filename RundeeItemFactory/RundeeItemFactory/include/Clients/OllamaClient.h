/**
 * @file OllamaClient.h
 * @brief Simple helper for calling Ollama from C++ (Windows, using HTTP API)
 * @author Haneul Lee (Rundee)
 * @date 2025-11-14
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides HTTP-based communication with Ollama LLM server.
 * Supports both simple single-call and retry-based execution modes.
 */

#pragma once

#include <string>
#include "Helpers/AppConfig.h"

/**
 * @class OllamaClient
 * @brief Static class for communicating with Ollama LLM server
 * 
 * Provides methods to send prompts to Ollama and receive JSON responses.
 * Uses HTTP API (WinHTTP on Windows) for communication.
 */
class OllamaClient
{
public:
    /**
     * @brief Simple helper function to call Ollama HTTP API
     * 
     * Makes a single HTTP request to Ollama with the given model and prompt.
     * Uses the provided OllamaSettings for connection configuration.
     * 
     * @param modelName Name of the LLM model (e.g., "llama3", "mistral")
     * @param prompt Prompt text to send to the LLM
     * @param settings Ollama connection settings (host, port, timeouts)
     * @return JSON response as a string, or empty string on error
     * 
     * @note This function does not retry on failure
     * @note Uses WinHTTP on Windows for HTTP communication
     * @see RunWithRetry for automatic retry logic
     */
    static std::string RunSimple(const std::string& modelName, 
                                 const std::string& prompt,
                                 const OllamaSettings& settings);

    /**
     * @brief Run LLM call with automatic retry logic
     * 
     * Attempts to call the LLM with automatic retry on failure.
     * Uses settings from AppConfig (loaded from config/rundee_config.json).
     * 
     * @param modelName Name of the LLM model (e.g., "llama3", "mistral")
     * @param prompt Prompt text to send to the LLM
     * @param maxRetries Maximum number of retry attempts (default: 3)
     * @param timeoutSeconds Timeout per attempt in seconds (default: 120, 0 = no timeout)
     * @return Response string, or empty string if all attempts failed
     * 
     * @note Automatically loads OllamaSettings from AppConfig
     * @note Retries with exponential backoff on failure
     * @note Logs retry attempts to std::cerr
     * @see RunSimple for single-call version
     */
    static std::string RunWithRetry(const std::string& modelName, 
                                    const std::string& prompt,
                                    int maxRetries = 3,
                                    int timeoutSeconds = 120);
};


