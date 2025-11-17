// ===============================
// Project Name: RundeeItemFactory
// File Name: StringUtils.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of string utility functions.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "StringUtils.h"

namespace StringUtils
{
    void TrimString(std::string& str)
    {
        size_t start = str.find_first_not_of(" \t\r\n");
        size_t end = str.find_last_not_of(" \t\r\n");
        if (start == std::string::npos)
        {
            str.clear();
            return;
        }
        str = str.substr(start, end - start + 1);
    }

    std::string CleanJsonArrayText(const std::string& input)
    {
        std::string s = input;
        TrimString(s);
        if (s.empty())
            return s;

        // Remove trailing whitespace and trailing comma
        int idx = static_cast<int>(s.size()) - 1;
        while (idx >= 0 && (s[idx] == ' ' || s[idx] == '\t' || s[idx] == '\r' || s[idx] == '\n'))
            --idx;

        if (idx >= 0 && s[idx] == ',')
        {
            s.erase(static_cast<size_t>(idx), 1);
        }

        // Close unclosed brackets
        int bracketBalance = 0;
        for (char c : s)
        {
            if (c == '[') bracketBalance++;
            else if (c == ']') bracketBalance--;
        }
        while (bracketBalance > 0)
        {
            s += "\n]";
            bracketBalance--;
        }

        return s;
    }

    std::string EscapeForCmd(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 8);
        for (char c : s)
        {
            if (c == '"')
                out += "\\\"";
            else
                out += c;
        }
        return out;
    }

    std::string EscapeString(const std::string& s)
    {
        std::string out;
        out.reserve(s.size() + 8);
        for (char c : s)
        {
            if (c == '\\')
                out += "\\\\";
            else if (c == '"')
                out += "\\\"";
            else
                out += c;
        }
        return out;
    }

    std::string NormalizeWhitespace(const std::string& s)
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
}


