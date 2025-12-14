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
#include <chrono>
#include <fstream>

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

        // Strip text before first '[' and after last ']'
        size_t first = s.find('[');
        size_t last = s.find_last_of(']');
        if (first != std::string::npos && last != std::string::npos && last >= first)
        {
            s = s.substr(first, last - first + 1);
        }

        // Remove/truncate at lines that are just "..." (truncated LLM output)
        auto truncateAtDotsLine = [](std::string& text)
        {
            size_t pos = 0;
            while (pos < text.size())
            {
                size_t lineStart = pos;
                size_t lineEnd = text.find('\n', lineStart);
                if (lineEnd == std::string::npos) lineEnd = text.size();

                std::string_view line(&text[lineStart], lineEnd - lineStart);
                size_t ls = 0;
                while (ls < line.size() && (line[ls] == ' ' || line[ls] == '\t' || line[ls] == '\r')) ls++;
                size_t le = line.size();
                while (le > ls && (line[le - 1] == ' ' || line[le - 1] == '\t' || line[le - 1] == '\r')) le--;
                std::string_view trimmed = line.substr(ls, le - ls);

                if (trimmed == "...")
                {
                    text.erase(lineStart);
                    break;
                }
                pos = (lineEnd < text.size()) ? (lineEnd + 1) : text.size();
            }
        };
        truncateAtDotsLine(s);

        // Remove trailing commas before a closing bracket (common after truncation)
        auto fixTrailingComma = [](std::string& text)
        {
            auto replace_all = [&](const std::string& from, const std::string& to)
            {
                size_t p = 0;
                while ((p = text.find(from, p)) != std::string::npos)
                {
                    text.replace(p, from.size(), to);
                }
            };
            replace_all(",\r\n]", "\r\n]");
            replace_all(",\n]", "\n]");
            replace_all(", ]", " ]");
            replace_all(",]", "]");
        };
        fixTrailingComma(s);

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
        int minBalance = 0;
        int openCount = 0;
        int closeCount = 0;
        for (char c : s)
        {
            if (c == '[')
            {
                ++openCount;
                ++bracketBalance;
            }
            else if (c == ']')
            {
                ++closeCount;
                --bracketBalance;
                if (bracketBalance < minBalance) minBalance = bracketBalance;
            }
        }

        if (minBalance < 0 || bracketBalance != 0)
        {
            // #region agent log
            static int dbgCount = 0;
            if (dbgCount < 20)
            {
                ++dbgCount;
                auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                auto sanitize = [](std::string v)
                {
                    for (char& ch : v)
                    {
                        if (ch == '"') ch = '\'';
                        else if (ch == '\\') ch = '/';
                    }
                    return v;
                };
                std::string first = sanitize(s.substr(0, std::min<size_t>(120, s.size())));
                std::string last = (s.size() > 120) ? sanitize(s.substr(s.size() - 120)) : first;
                std::ofstream dbg("d:\\_VisualStudioProjects\\_Rundee_RundeeItemFactory\\.cursor\\debug.log", std::ios::app);
                if (dbg.is_open())
                {
                    dbg << R"({"sessionId":"debug-session","runId":"json-clean","hypothesisId":"H1","location":"StringUtils.cpp","message":"bracket-balance","data":{"open":)"
                        << openCount << R"(,"close":)" << closeCount << R"(,"minBalance":)" << minBalance
                        << R"(,"balance":)" << bracketBalance << R"(,"len":)" << s.size()
                        << R"(,"first":")" << first << R"(","last":")" << last << R"("},"timestamp":)" << ts << "})" << "\n";
                }
            }
            // #endregion
        }

        if (bracketBalance > 0)
        {
            int lastIdx = static_cast<int>(s.size()) - 1;
            while (lastIdx >= 0 && (s[lastIdx] == ' ' || s[lastIdx] == '\t' || s[lastIdx] == '\r' || s[lastIdx] == '\n'))
                --lastIdx;
            if (lastIdx >= 0 && s[lastIdx] == ',')
            {
                s.erase(static_cast<size_t>(lastIdx), 1);
            }
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

    std::string StripAnsiEscapeCodes(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());

        enum class State
        {
            Normal,
            EscSeen,
            Csi,
            Osc
        };

        State state = State::Normal;
        std::string oscBuffer;

        for (size_t i = 0; i < s.size(); ++i)
        {
            unsigned char c = static_cast<unsigned char>(s[i]);

            switch (state)
            {
            case State::Normal:
                if (c == 0x1B) // ESC
                {
                    state = State::EscSeen;
                }
                else
                {
                    out += s[i];
                }
                break;

            case State::EscSeen:
                if (c == '[')
                {
                    state = State::Csi;
                }
                else if (c == ']')
                {
                    state = State::Osc;
                    oscBuffer.clear();
                }
                else
                {
                    // Not CSI/OSC, drop this char as part of ESC sequence
                    state = State::Normal;
                }
                break;

            case State::Csi:
                if (c >= 0x40 && c <= 0x7E)
                {
                    state = State::Normal;
                }
                // otherwise stay in CSI
                break;

            case State::Osc:
                if (c == 0x07) // BEL terminates OSC
                {
                    state = State::Normal;
                }
                else if (c == 0x1B && i + 1 < s.size() && s[i + 1] == '\\')
                {
                    // ESC \ terminates OSC
                    state = State::Normal;
                    ++i; // skip '\'
                }
                // otherwise keep consuming until terminator
                break;
            }
        }

        return out;
    }

    std::string RemoveControlCharacters(const std::string& s)
    {
        std::string out;
        out.reserve(s.size());

        for (unsigned char c : s)
        {
            if (c >= 32 || c == '\n' || c == '\r' || c == '\t')
            {
                out += static_cast<char>(c);
            }
        }

        return out;
    }
}


