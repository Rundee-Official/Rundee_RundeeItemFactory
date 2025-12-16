// ===============================
// Project Name: RundeeItemFactory
// File Name: AppConfig.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Lightweight config loader for shared defaults (Ollama host/port, timeouts, retries).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Helpers/AppConfig.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace
{
    using json = nlohmann::json;

    OllamaSettings g_settings{};
    bool g_loaded = false;
    std::string g_loadedPath;

    std::string ReadAllText(const std::filesystem::path& path)
    {
        std::ifstream ifs(path, std::ios::in | std::ios::binary);
        if (!ifs.is_open())
        {
            return {};
        }
        return std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }

    void SetIfPresent(const json& j, const char* key, int& outValue)
    {
        if (j.contains(key) && j[key].is_number_integer())
        {
            outValue = j[key].get<int>();
        }
    }

    void SetIfPresent(const json& j, const char* key, std::string& outValue)
    {
        if (j.contains(key) && j[key].is_string())
        {
            outValue = j[key].get<std::string>();
        }
    }
}

void AppConfig::EnsureLoaded()
{
    if (!g_loaded)
    {
        LoadFromDefaultLocation();
    }
}

void AppConfig::LoadFromDefaultLocation()
{
    if (g_loaded)
        return;

    try
    {
        std::filesystem::path base = std::filesystem::current_path();
        std::filesystem::path defaultPath = base / "config" / "rundee_config.json";

        if (!std::filesystem::exists(defaultPath))
        {
            // No config file is fine; keep defaults.
            g_loaded = true;
            return;
        }

        std::string jsonText = ReadAllText(defaultPath);
        if (jsonText.empty())
        {
            std::cerr << "[AppConfig] Config file is empty: " << defaultPath.string() << "\n";
            g_loaded = true;
            return;
        }

        ApplyJson(jsonText, defaultPath.string());
        g_loaded = true;
        g_loadedPath = defaultPath.string();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[AppConfig] Failed to load default config: " << ex.what() << "\n";
        g_loaded = true;
    }
}

void AppConfig::LoadFromPath(const std::string& path)
{
    // Explicit load always overwrites defaults if successful.
    try
    {
        std::filesystem::path p(path);
        if (!std::filesystem::exists(p))
        {
            std::cerr << "[AppConfig] Config path not found: " << path << "\n";
            return;
        }

        std::string jsonText = ReadAllText(p);
        if (jsonText.empty())
        {
            std::cerr << "[AppConfig] Config is empty: " << path << "\n";
            return;
        }

        ApplyJson(jsonText, path);
        g_loaded = true;
        g_loadedPath = path;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[AppConfig] Failed to load config from " << path << ": " << ex.what() << "\n";
    }
}

void AppConfig::ApplyJson(const std::string& jsonText, const std::string& sourceLabel)
{
    try
    {
        json root = json::parse(jsonText);

        if (root.contains("ollama") && root["ollama"].is_object())
        {
            const auto& o = root["ollama"];
            SetIfPresent(o, "host", g_settings.host);
            SetIfPresent(o, "port", g_settings.port);
            SetIfPresent(o, "maxRetries", g_settings.maxRetries);
            SetIfPresent(o, "requestTimeoutSeconds", g_settings.requestTimeoutSeconds);
            SetIfPresent(o, "connectTimeoutMs", g_settings.connectTimeoutMs);
            SetIfPresent(o, "sendTimeoutMs", g_settings.sendTimeoutMs);
            SetIfPresent(o, "receiveTimeoutMs", g_settings.receiveTimeoutMs);
        }

        std::cout << "[AppConfig] Loaded config from " << sourceLabel << "\n";
    }
    catch (const std::exception& ex)
    {
        std::cerr << "[AppConfig] Failed to parse config (" << sourceLabel << "): " << ex.what() << "\n";
    }
}

const OllamaSettings& AppConfig::GetOllamaSettings()
{
    EnsureLoaded();
    return g_settings;
}

