// ===============================
// Project Name: RundeeItemFactory
// File Name: OllamaClient.h
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Simple helper for calling Ollama from C++ (Windows, using _popen).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#pragma once

#include <string>

class OllamaClient
{
public:
    // Simple helper function:
    // Runs ollama run command with the given model name and prompt,
    // Returns the entire stdout as a string.
    // Returns empty string on error.
    static std::string RunSimple(const std::string& modelName, const std::string& prompt);
};


