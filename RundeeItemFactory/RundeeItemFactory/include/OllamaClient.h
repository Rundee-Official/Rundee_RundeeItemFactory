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
    // 가장 단순한 버전:
    // 지정한 모델 이름과 프롬프트를 사용해서 ollama run 실행,
    // stdout 전체를 문자열로 리턴.
    // 에러 발생 시 빈 문자열 리턴.
    static std::string RunSimple(const std::string& modelName, const std::string& prompt);
};
