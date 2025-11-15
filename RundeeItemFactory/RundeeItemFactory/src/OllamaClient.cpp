// ===============================
// Project Name: RundeeItemFactory
// File Name: OllamaClient.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Simple helper for calling Ollama from C++ (Windows).
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "OllamaClient.h"
#include <iostream>
#include <cstdio> 

static std::string NormalizeWhitespace(const std::string& s)
{
    std::string out;
    out.reserve(s.size());

    for (char c : s)
    {
        if (c == '\n' || c == '\r' || c == '\t')
            out += ' ';
        else
            out += c;
    }
    return out;
}

// 아주 러프하게 쿼트만 최소한 escape
static std::string EscapeForCmd(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s)
    {
        if (c == '"')
            out += "\\\"";   // " → \"
        else
            out += c;
    }
    return out;
}

std::string OllamaClient::RunSimple(const std::string& modelName, const std::string& prompt)
{
    std::string flatPrompt = NormalizeWhitespace(prompt);
    std::string escapedPrompt = EscapeForCmd(flatPrompt);
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
