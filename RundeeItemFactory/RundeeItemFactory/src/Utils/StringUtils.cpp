// ===============================
// Project Name: RundeeItemFactory
// File Name: StringUtils.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-15
// Description: Implementation of string utility functions.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "Utils/StringUtils.h"

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

    std::string FixCommonJsonErrors(const std::string& input)
    {
        std::string s = input;
        
        // Fix )key": -> "key": (missing opening quote, e.g., )rarity": -> "rarity":)
        // Pattern: ) followed by alphanumeric/underscore key name, then ": or "
        size_t pos = 0;
        while (pos < s.length())
        {
            if (s[pos] == ')' && pos + 1 < s.length())
            {
                // Check if next character is alphanumeric or underscore (start of key name)
                char next = s[pos + 1];
                if ((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z') || next == '_')
                {
                    // Find the end of the key name
                    size_t keyEnd = pos + 1;
                    while (keyEnd < s.length() && 
                           ((s[keyEnd] >= 'a' && s[keyEnd] <= 'z') ||
                            (s[keyEnd] >= 'A' && s[keyEnd] <= 'Z') ||
                            (s[keyEnd] >= '0' && s[keyEnd] <= '9') ||
                            s[keyEnd] == '_'))
                    {
                        keyEnd++;
                    }
                    
                    // Check if there's ": or " after the key name
                    if (keyEnd < s.length())
                    {
                        size_t checkPos = keyEnd;
                        // Skip whitespace
                        while (checkPos < s.length() && (s[checkPos] == ' ' || s[checkPos] == '\t'))
                            checkPos++;
                        
                        // Check for ": pattern (most common)
                        if (checkPos + 1 < s.length() && s[checkPos] == '"' && s[checkPos + 1] == ':')
                        {
                            // Replace )key": with "key":
                            std::string keyName = s.substr(pos + 1, keyEnd - pos - 1);
                            std::string afterKey = s.substr(keyEnd, checkPos - keyEnd);
                            s.replace(pos, checkPos + 2 - pos, "\"" + keyName + "\":" + s.substr(checkPos + 2));
                            pos = checkPos + 2; // Skip past the fixed key
                            continue;
                        }
                        // Check for " pattern (less common)
                        else if (checkPos < s.length() && s[checkPos] == '"')
                        {
                            // Replace )key" with "key"
                            std::string keyName = s.substr(pos + 1, keyEnd - pos - 1);
                            std::string afterKey = s.substr(keyEnd, checkPos - keyEnd);
                            s.replace(pos, checkPos - pos, "\"" + keyName + "\"" + afterKey);
                            pos = checkPos + 1; // Skip past the fixed key
                            continue;
                        }
                    }
                }
            }
            pos++;
        }
        
        // Fix spaces in IDs (e.g., "fresh_ juice_box" -> "fresh_juice_box")
        pos = 0;
        while ((pos = s.find("\"id\": \"", pos)) != std::string::npos)
        {
            size_t idStart = pos + 7; // After "id": "
            size_t idEnd = s.find("\"", idStart);
            if (idEnd != std::string::npos)
            {
                std::string idValue = s.substr(idStart, idEnd - idStart);
                // Replace spaces with underscores in ID
                for (size_t i = 0; i < idValue.length(); i++)
                {
                    if (idValue[i] == ' ')
                    {
                        idValue[i] = '_';
                    }
                }
                s.replace(idStart, idEnd - idStart, idValue);
                pos = idStart + idValue.length();
            }
            else
            {
                pos++;
            }
        }
        
        return s;
    }

    std::string CleanJsonArrayText(const std::string& input)
    {
        std::string s = input;
        TrimString(s);
        if (s.empty())
            return s;

        // Fix common JSON errors first
        s = FixCommonJsonErrors(s);

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


