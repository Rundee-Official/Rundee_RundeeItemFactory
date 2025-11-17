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

std::string OllamaClient::RunSimple(const std::string& modelName, const std::string& prompt)
{
    std::string flatPrompt = StringUtils::NormalizeWhitespace(prompt);
    std::string escapedPrompt = StringUtils::EscapeForCmd(flatPrompt);
    std::string command = "ollama run " + modelName +
        " \"" + escapedPrompt + "\"";

    std::cout << "[OllamaClient] Running command: " << command << "\n";

    std::string result;

    // _popen 으로 명령 실행하고 stdout 읽기
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


