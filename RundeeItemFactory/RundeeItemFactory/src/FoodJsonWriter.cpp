// ===============================
// Project Name: RundeeItemFactory
// File Name: FoodJsonWriter.cpp
// Author: Haneul Lee (Rundee)
// Created Date: 2025-11-14
// Description: Implementation for writing food items to a JSON file.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

#include "FoodJsonWriter.h"
#include <fstream>
#include <iostream>

static std::string EscapeString(const std::string& s)
{
    // 아주 단순한 버전: 큰따옴표, 역슬래시만 최소한 escape
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

bool FoodJsonWriter::WriteToFile(const std::vector<ItemFoodData>& items,
    const std::string& outputPath)
{
    std::ofstream ofs(outputPath);
    if (!ofs.is_open())
    {
        std::cerr << "[FoodJsonWriter] Failed to open file: " << outputPath << "\n";
        return false;
    }

    ofs << "[\n";

    for (size_t i = 0; i < items.size(); ++i)
    {
        const auto& it = items[i];

        ofs << "  {\n";
        ofs << "    \"id\": \"" << EscapeString(it.id) << "\",\n";
        ofs << "    \"displayName\": \"" << EscapeString(it.displayName) << "\",\n";
        ofs << "    \"category\": \"" << EscapeString(it.category) << "\",\n";
        ofs << "    \"rarity\": \"" << EscapeString(it.rarity) << "\",\n";
        ofs << "    \"maxStack\": " << it.maxStack << ",\n";
        ofs << "    \"hungerRestore\": " << it.hungerRestore << ",\n";
        ofs << "    \"thirstRestore\": " << it.thirstRestore << ",\n";
        ofs << "    \"healthRestore\": " << it.healthRestore << ",\n";
        ofs << "    \"spoils\": " << (it.spoils ? "true" : "false") << ",\n";
        ofs << "    \"spoilTimeMinutes\": " << it.spoilTimeMinutes << ",\n";
        ofs << "    \"description\": \"" << EscapeString(it.description) << "\"\n";
        ofs << "  }";

        if (i + 1 < items.size())
            ofs << ",";

        ofs << "\n";
    }

    ofs << "]\n";

    std::cout << "[FoodJsonWriter] Wrote " << items.size()
        << " items to JSON file: " << outputPath << "\n";

    return true;
}
