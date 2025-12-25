/**
 * @file AppConfig.h
 * @brief Lightweight config loader for shared defaults (Ollama host/port, timeouts, retries)
 * @author Haneul Lee (Rundee)
 * @date 2025-12-10
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 * 
 * Provides configuration management for Ollama connection settings.
 * Loads settings from config/rundee_config.json or a custom path.
 */

#pragma once

#include <string>

/**
 * @struct OllamaSettings
 * @brief Configuration structure for Ollama LLM connection
 * 
 * Contains all settings needed to connect to and communicate with Ollama server.
 */
struct OllamaSettings
{
    /**
     * @brief Ollama server hostname
     * 
     * Default: "localhost"
     */
    std::string host = "localhost";

    /**
     * @brief Ollama server port
     * 
     * Default: 11434 (standard Ollama port)
     */
    int port = 11434;

    /**
     * @brief Maximum number of retry attempts
     * 
     * Default: 3
     */
    int maxRetries = 3;

    /**
     * @brief Overall request timeout per attempt in seconds
     * 
     * Default: 120 seconds
     */
    int requestTimeoutSeconds = 120;

    /**
     * @brief DNS/connect timeout in milliseconds
     * 
     * Default: 5000 ms
     */
    int connectTimeoutMs = 5000;

    /**
     * @brief Upload timeout in milliseconds
     * 
     * Default: 120000 ms (120 seconds)
     */
    int sendTimeoutMs = 120000;

    /**
     * @brief Download timeout in milliseconds
     * 
     * Default: 120000 ms (120 seconds)
     */
    int receiveTimeoutMs = 120000;
};

/**
 * @class AppConfig
 * @brief Static class for loading and accessing application configuration
 * 
 * Provides methods to load configuration from JSON files and access
 * Ollama settings. Thread-safe and safe to call multiple times.
 */
class AppConfig
{
public:
    /**
     * @brief Load config from default location
     * 
     * Loads configuration from config/rundee_config.json relative to the working directory.
     * Safe to call multiple times; first successful load wins.
     * 
     * @note If the config file doesn't exist, defaults are used
     * @note Parse failures are logged but don't throw exceptions
     */
    static void LoadFromDefaultLocation();

    /**
     * @brief Load config from explicit path
     * 
     * Loads configuration from the specified JSON file path.
     * Non-existent files or parse failures are logged but ignored.
     * 
     * @param path Path to JSON configuration file
     */
    static void LoadFromPath(const std::string& path);

    /**
     * @brief Access loaded Ollama settings
     * 
     * Returns the loaded Ollama settings, or default settings if loading failed.
     * Automatically ensures configuration is loaded before returning.
     * 
     * @return Reference to OllamaSettings structure
     */
    static const OllamaSettings& GetOllamaSettings();

private:
    /**
     * @brief Ensure configuration is loaded
     * 
     * Internal method that ensures configuration has been loaded at least once.
     * Calls LoadFromDefaultLocation if not yet loaded.
     */
    static void EnsureLoaded();

    /**
     * @brief Apply JSON configuration
     * 
     * Parses JSON text and applies settings to the global configuration.
     * 
     * @param jsonText JSON configuration text
     * @param sourceLabel Label for logging (e.g., file path)
     */
    static void ApplyJson(const std::string& jsonText, const std::string& sourceLabel);
};
