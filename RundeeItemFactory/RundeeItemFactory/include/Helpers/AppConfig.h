// ===============================
// Project Name: RundeeItemFactory
// File Name: AppConfig.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Lightweight config loader for shared defaults (Ollama host/port, timeouts, retries).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

// Standard Library Includes
#include <string>

// ============================================================================
// SECTION 1: Data Structures
// ============================================================================

struct OllamaSettings
{
    std::string host = "localhost";
    int port = 11434;
    int maxRetries = 3;
    int requestTimeoutSeconds = 120;   // overall request timeout per attempt
    int connectTimeoutMs = 5000;       // DNS/connect timeout
    int sendTimeoutMs = 120000;        // upload timeout
    int receiveTimeoutMs = 120000;     // download timeout
};

// ============================================================================
// SECTION 2: AppConfig Class
// ============================================================================

class AppConfig
{
public:
    // Load config from default location (config/rundee_config.json relative to working directory).
    // Safe to call multiple times; first successful load wins.
    static void LoadFromDefaultLocation();

    // Load config from explicit path. Non-existent or parse failures will be logged but ignored.
    static void LoadFromPath(const std::string& path);

    // Access loaded settings (or defaults if loading failed).
    static const OllamaSettings& GetOllamaSettings();

private:
    static void EnsureLoaded();
    static void ApplyJson(const std::string& jsonText, const std::string& sourceLabel);
};

