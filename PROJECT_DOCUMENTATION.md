# RundeeItemFactory - 프로젝트 전체 문서

## 📋 목차

1. [프로젝트 개요](#프로젝트-개요)
2. [시스템 아키텍처](#시스템-아키텍처)
3. [C++ 프로젝트 구조](#c-프로젝트-구조)
4. [Unity 프로젝트 구조](#unity-프로젝트-구조)
5. [핵심 컴포넌트 상세](#핵심-컴포넌트-상세)
6. [데이터 흐름](#데이터-흐름)
7. [사용 시나리오](#사용-시나리오)
8. [기술 스택](#기술-스택)

---

## 프로젝트 개요

**RundeeItemFactory**는 LLM(Large Language Model)을 활용하여 게임 아이템 데이터를 자동 생성하는 통합 시스템입니다. 로컬 LLM 서버(Ollama)를 사용하여 완전히 오프라인에서 작동하며, Unity Editor와 통합된 GUI를 제공합니다.

### 핵심 특징

- **동적 프로필 시스템**: 코드 수정 없이 커스텀 아이템 구조 정의
- **로컬 LLM 통합**: Ollama를 통한 완전 오프라인 작동
- **Unity Editor 통합**: GUI 기반 프로필 관리 및 아이템 생성
- **자동화된 워크플로우**: 프로필 생성 → 아이템 생성 → Unity ScriptableObject 변환

### 프로젝트 구성

프로젝트는 두 가지 주요 컴포넌트로 구성됩니다:

1. **C++ 실행 파일** (`RundeeItemFactory.exe`): LLM과 통신하여 아이템 JSON 생성
2. **Unity Editor 패키지**: 프로필 관리, GUI, ScriptableObject 변환

---

## 시스템 아키텍처

### 전체 구조

```
┌─────────────────────────────────────────────────────────────┐
│                    Unity Editor (C#)                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  ItemFactoryMainWindow                                │  │
│  │  - 프로필 관리 (Item/Player Profile Manager)        │  │
│  │  - 아이템 생성 UI                                    │  │
│  │  - JSON Import                                       │  │
│  └──────────────────┬───────────────────────────────────┘  │
│                     │ 프로필 JSON 파일                      │
│                     ▼                                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  DynamicItemImporter                                  │  │
│  │  - JSON → ScriptableObject 변환                      │  │
│  └──────────────────────────────────────────────────────┘  │
└──────────────────────┬──────────────────────────────────────┘
                       │ 프로필 JSON + 명령줄 인자
                       ▼
┌─────────────────────────────────────────────────────────────┐
│              C++ Executable (RundeeItemFactory.exe)         │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  ItemGenerator                                        │  │
│  │  - 프로필 로드                                       │  │
│  │  - 프롬프트 생성                                     │  │
│  │  - LLM 호출                                          │  │
│  │  - JSON 생성                                         │  │
│  └──────────────────┬───────────────────────────────────┘  │
│                     │ HTTP 요청                             │
│                     ▼                                       │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  OllamaClient                                         │  │
│  │  - Ollama API 통신                                    │  │
│  └──────────────────┬───────────────────────────────────┘  │
└──────────────────────┼──────────────────────────────────────┘
                       │
                       ▼
              ┌─────────────────┐
              │  Ollama Server  │
              │  (Local LLM)    │
              └─────────────────┘
```

### 데이터 흐름

1. **프로필 생성 단계** (Unity Editor)
   - 사용자가 Item Profile과 Player Profile을 Unity Editor에서 생성
   - JSON 파일로 `ItemProfiles/`, `PlayerProfiles/` 디렉토리에 저장

2. **아이템 생성 단계** (C++ Executable)
   - Unity Editor에서 `RundeeItemFactory.exe` 실행
   - 프로필 JSON 파일 로드
   - DynamicPromptBuilder가 프로필 데이터로 프롬프트 생성
   - OllamaClient를 통해 LLM 호출
   - 생성된 아이템을 JSON 파일로 저장 (`ItemJson/`)

3. **Import 단계** (Unity Editor)
   - 생성된 JSON 파일을 Unity Editor에서 Import
   - DynamicItemImporter가 JSON을 ScriptableObject로 변환
   - `Assets/Resources/RundeeItemFactory/`에 저장

---

## C++ 프로젝트 구조

### 디렉토리 구조

```
RundeeItemFactory/
├── include/                    # 헤더 파일
│   ├── Data/                   # 데이터 구조체
│   │   ├── ItemProfile.h       # 아이템 프로필 구조
│   │   ├── ItemProfileManager.h # 프로필 로더
│   │   ├── PlayerProfile.h     # 플레이어 프로필 구조
│   │   ├── PlayerProfileManager.h
│   │   └── ItemDataBase.h      # 기본 아이템 데이터
│   ├── Generators/             # 아이템 생성 로직
│   │   ├── ItemGenerator.h     # 메인 생성기
│   │   └── ItemGeneratorRegistry.h # ID 레지스트리
│   ├── Clients/                # 외부 서비스 클라이언트
│   │   └── OllamaClient.h      # Ollama API 클라이언트
│   ├── Prompts/                # 프롬프트 생성
│   │   ├── DynamicPromptBuilder.h # 동적 프롬프트 빌더
│   │   ├── PromptTemplateLoader.h
│   │   └── CustomPreset.h      # (Deprecated)
│   ├── Parsers/                # JSON 파싱
│   │   └── DynamicItemJsonParser.h
│   ├── Writers/                # JSON 쓰기
│   │   └── DynamicItemJsonWriter.h
│   ├── Helpers/                # 유틸리티
│   │   ├── CommandLineParser.h # 명령줄 인자 파싱
│   │   ├── AppConfig.h         # 설정 로드
│   │   └── ItemGenerateParams.h
│   └── Utils/                  # 유틸리티 함수
│       ├── StringUtils.h
│       └── JsonUtils.h
├── src/                        # 구현 파일
│   └── (include와 동일한 구조)
├── config/
│   └── rundee_config.json      # Ollama 설정
└── RundeeItemFactory.cpp       # 메인 진입점
```

### 주요 컴포넌트

#### 1. ItemGenerator (핵심 생성 엔진)

**위치**: `src/Generators/ItemGenerator.cpp`

**역할**:
- 프로필 로드 및 검증
- 프롬프트 생성 (DynamicPromptBuilder 사용)
- LLM 호출 (OllamaClient 사용)
- 생성된 아이템 검증 및 저장
- 중복 ID 방지 (ItemGeneratorRegistry 사용)

**주요 메서드**:
```cpp
int GenerateWithLLM(const CommandLineArgs& args)
```

**처리 흐름**:
1. Item Profile과 Player Profile 로드
2. 기존 아이템 파일 읽기 (중복 방지)
3. ID 레지스트리 로드
4. 배치 단위로 아이템 생성 (병렬 처리 지원)
5. LLM 응답 파싱 및 검증
6. JSON 파일에 저장

#### 2. DynamicPromptBuilder (프롬프트 생성)

**위치**: `src/Prompts/DynamicPromptBuilder.cpp`

**역할**:
- Item Profile의 모든 필드 정의를 프롬프트로 변환
- Validation 규칙 포함
- Player Profile의 통계 설정 포함
- World Context (customContext) 포함
- 기존 ID 목록 포함 (중복 방지)

**핵심 메서드**:
```cpp
std::string BuildPromptFromProfile(
    const ItemProfile& profile,
    const PlayerProfile& playerProfile,
    const std::vector<std::string>& existingIds
)
```

**프롬프트 구조**:
```
World Context / Background
[ItemProfile.customContext]

Item Structure
[모든 필드 정의, 타입, 검증 규칙]

Player Context
[PlayerProfile의 통계 설정]

Existing Items
[기존 ID 목록 - 중복 방지]

Generation Instructions
[생성할 아이템 수, 형식 등]
```

#### 3. OllamaClient (LLM 통신)

**위치**: `src/Clients/OllamaClient.cpp`

**역할**:
- Ollama HTTP API와 통신
- 프롬프트 전송 및 응답 수신
- 재시도 로직 (지수 백오프)
- 타임아웃 처리

**주요 메서드**:
```cpp
bool GenerateCompletion(
    const std::string& prompt,
    const std::string& model,
    std::string& response,
    int maxRetries = 3
)
```

**API 엔드포인트**: `POST http://localhost:11434/api/generate`

#### 4. ItemProfileManager / PlayerProfileManager

**위치**: `src/Data/ItemProfileManager.cpp`, `src/Data/PlayerProfileManager.cpp`

**역할**:
- JSON 파일에서 프로필 로드
- 프로필 검증
- 기본 프로필 찾기

**주요 메서드**:
```cpp
bool LoadProfile(const std::string& profileId, ItemProfile& profile)
bool LoadDefaultProfile(ItemType itemType, ItemProfile& profile)
```

#### 5. CommandLineParser

**위치**: `src/Helpers/CommandLineParser.cpp`

**역할**:
- 명령줄 인자 파싱
- 기본값 설정
- 인자 검증

**지원 인자**:
- `--mode`: 생성 모드 (llm)
- `--itemType`: 아이템 타입 (레거시, 프로필 기반으로 대체됨)
- `--model`: Ollama 모델 이름
- `--count`: 생성할 아이템 수
- `--profile`: Item Profile ID
- `--playerProfile`: Player Profile ID
- `--out`: 출력 파일명

---

## Unity 프로젝트 구조

### 디렉토리 구조

```
Assets/RundeeItemFactory/
├── Editor/                     # Unity Editor 스크립트
│   ├── ItemFactoryMainWindow.cs      # 메인 윈도우
│   ├── ItemProfileManagerWindow.cs   # Item Profile 관리
│   ├── PlayerProfileManagerWindow.cs # Player Profile 관리
│   ├── DynamicItemImporter.cs       # JSON → ScriptableObject
│   ├── ExecutableDownloader.cs      # .exe 다운로드
│   ├── ErrorHandler.cs              # 에러 처리
│   ├── InputValidator.cs            # 입력 검증
│   └── ItemFactorySettings.cs       # 설정 저장
├── Runtime/                    # 런타임 스크립트
│   ├── ItemProfile.cs              # Item Profile C# 클래스
│   └── DynamicItemDataSO.cs         # ScriptableObject 베이스
├── ItemProfiles/               # Item Profile JSON 파일
├── PlayerProfiles/             # Player Profile JSON 파일
├── ItemJson/                   # 생성된 아이템 JSON
├── Registry/                   # ID 레지스트리
└── RundeeItemFactory.exe       # C++ 실행 파일
```

### 주요 컴포넌트

#### 1. ItemFactoryMainWindow (메인 UI)

**위치**: `Editor/ItemFactoryMainWindow.cs`

**역할**:
- 통합 UI 제공 (4개 탭)
- Ollama 설치 및 검증
- .exe 다운로드 및 관리
- 아이템 생성 실행
- 프로필 선택 및 관리

**탭 구조**:
1. **Item Factory**: 아이템 생성 메인 탭
2. **Player Profile Manager**: Player Profile 생성/편집/삭제
3. **Item Profile Manager**: Item Profile 생성/편집/삭제
4. **Import Item From Json**: JSON 파일 Import

**주요 기능**:
- **Quick Setup**: Ollama와 .exe 자동 설치
- **Advanced Setup Options**: 개별 설치 옵션
- **Run Diagnostics**: 시스템 상태 진단
- **Model Management**: Ollama 모델 설치 및 선택
- **Status Indicators**: LED 기반 상태 표시 (초록/빨강)

#### 2. ItemProfileManagerWindow

**위치**: `Editor/ItemProfileManagerWindow.cs`

**역할**:
- Item Profile 생성/편집/삭제
- 필드 정의 UI
- Validation 규칙 설정
- World Context 편집

**주요 기능**:
- 필드 추가/삭제/순서 변경
- 필드 타입 선택 (String, Integer, Float, Boolean, Array, Object)
- Validation 규칙 설정 (min/max, allowed values, required)
- 카테고리별 필드 그룹화

#### 3. PlayerProfileManagerWindow

**위치**: `Editor/PlayerProfileManagerWindow.cs`

**역할**:
- Player Profile 생성/편집/삭제
- 플레이어 통계 설정
- Stat Sections 관리

**주요 기능**:
- 기본 통계 설정 (maxHunger, maxThirst, maxHealth, etc.)
- Stat Sections 추가 (중첩된 통계 구조)
- 프로필 메타데이터 관리

#### 4. DynamicItemImporter

**위치**: `Editor/DynamicItemImporter.cs`

**역할**:
- JSON 파일을 ScriptableObject로 변환
- Item Profile 기반 동적 필드 매핑
- Unity AssetDatabase 통합

**주요 메서드**:
```csharp
void ImportJsonFile(string jsonPath, ItemProfile profile)
```

**처리 흐름**:
1. JSON 파일 파싱
2. Item Profile 로드
3. 각 아이템에 대해 ScriptableObject 생성
4. 프로필의 필드 정의에 따라 동적으로 속성 설정
5. `Assets/Resources/RundeeItemFactory/[Type]Items/`에 저장

#### 5. ExecutableDownloader

**위치**: `Editor/ExecutableDownloader.cs`

**역할**:
- GitHub Releases에서 .exe 다운로드
- 다운로드 진행률 표시
- 다운로드 후 검증

**주요 기능**:
- UnityWebRequest를 사용한 비동기 다운로드
- 진행률 콜백
- 에러 처리 및 재시도

#### 6. ErrorHandler

**위치**: `Editor/ErrorHandler.cs`

**역할**:
- 중앙화된 에러 처리
- 사용자 친화적 에러 메시지
- 해결 방법 제시

**에러 타입**:
- `OllamaNotInstalled`
- `OllamaNotRunning`
- `ExecutableNotInstalled`
- `ProfileNotFound`
- `GenerationFailed`

---

## 핵심 컴포넌트 상세

### Dynamic Profile System

프로젝트의 핵심은 **Dynamic Profile System**입니다. 이 시스템을 통해 코드 수정 없이 완전히 커스텀 아이템 구조를 정의할 수 있습니다.

#### Item Profile 구조

```json
{
  "id": "realistic_firearms",
  "displayName": "Realistic Firearms",
  "description": "Item profile for generating realistic firearms",
  "itemTypeName": "Weapon",
  "version": 1,
  "isDefault": false,
  "customContext": "Generate realistic firearms based on real-world weapons...",
  "fields": [
    {
      "name": "id",
      "type": 0,  // String
      "displayName": "ID",
      "description": "Unique item identifier",
      "category": "Identity",
      "displayOrder": 0,
      "defaultValue": "",
      "validation": {
        "isRequired": true,
        "minLength": 1,
        "maxLength": 100
      }
    },
    {
      "name": "minDamage",
      "type": 1,  // Integer
      "displayName": "Min Damage",
      "description": "Minimum damage per shot",
      "category": "Weapon Stats",
      "displayOrder": 8,
      "defaultValue": "10",
      "validation": {
        "isRequired": true,
        "minValue": 1.0,
        "maxValue": 100.0
      }
    }
  ],
  "metadata": {},
  "playerSettings": {}
}
```

#### Player Profile 구조

```json
{
  "id": "example_player",
  "displayName": "Example Player Profile",
  "description": "A basic player profile",
  "version": 1,
  "isDefault": false,
  "playerSettings": {
    "maxHunger": 100,
    "maxThirst": 100,
    "maxHealth": 100,
    "maxStamina": 100,
    "maxWeight": 50000,
    "maxEnergy": 100
  },
  "statSections": []
}
```

### 프롬프트 생성 로직

`DynamicPromptBuilder`는 프로필 데이터를 기반으로 LLM에 전송할 프롬프트를 동적으로 생성합니다.

**프롬프트 구성 요소**:

1. **World Context**: Item Profile의 `customContext` 필드
2. **Item Structure**: 모든 필드 정의, 타입, 검증 규칙
3. **Player Context**: Player Profile의 통계 설정
4. **Existing Items**: 중복 방지를 위한 기존 ID 목록
5. **Generation Instructions**: 생성할 아이템 수, 출력 형식

이를 통해 LLM은 완전한 컨텍스트를 가지고 아이템을 생성할 수 있습니다.

### ID 레지스트리 시스템

`ItemGeneratorRegistry`는 생성된 아이템의 ID를 추적하여 중복을 방지합니다.

**작동 방식**:
1. `Registry/` 디렉토리에 타입별 레지스트리 파일 저장
2. 아이템 생성 전 기존 ID 로드
3. 프롬프트에 기존 ID 목록 포함
4. 생성 후 새 ID를 레지스트리에 추가

**레지스트리 파일 형식**:
```json
{
  "itemType": "Weapon",
  "ids": [
    "Weapon_ak47",
    "Weapon_glock17",
    "Weapon_m4a1"
  ]
}
```

---

## 데이터 흐름

### 전체 워크플로우

```
1. 프로필 생성 (Unity Editor)
   └─> ItemProfileManagerWindow에서 Item Profile 생성
   └─> PlayerProfileManagerWindow에서 Player Profile 생성
   └─> JSON 파일로 저장 (ItemProfiles/, PlayerProfiles/)

2. 아이템 생성 (Unity Editor → C++ Executable)
   └─> ItemFactoryMainWindow에서 프로필 선택
   └─> "Generate Items" 버튼 클릭
   └─> RundeeItemFactory.exe 실행 (명령줄)
   └─> 프로필 JSON 로드
   └─> 프롬프트 생성
   └─> Ollama API 호출
   └─> JSON 응답 파싱
   └─> ItemJson/에 저장

3. Import (Unity Editor)
   └─> "Import Item From Json" 탭에서 JSON 선택
   └─> DynamicItemImporter 실행
   └─> ScriptableObject 생성
   └─> Resources/에 저장
```

### 상세 데이터 흐름

#### 1. 프로필 생성 → 저장

```
Unity Editor UI
    ↓ (사용자 입력)
ItemProfile C# 객체
    ↓ (JSON 직렬화)
ItemProfiles/{profile_id}.json
```

#### 2. 아이템 생성 요청

```
Unity Editor
    ↓ (명령줄 인자 구성)
Process.Start("RundeeItemFactory.exe", args)
    ↓
C++ Executable
    ↓ (프로필 로드)
ItemProfileManager::LoadProfile()
    ↓ (프롬프트 생성)
DynamicPromptBuilder::BuildPromptFromProfile()
    ↓ (LLM 호출)
OllamaClient::GenerateCompletion()
    ↓ (응답 파싱)
DynamicItemJsonParser::ParseItems()
    ↓ (저장)
DynamicItemJsonWriter::WriteItems()
    ↓
ItemJson/items.json
```

#### 3. JSON Import

```
ItemJson/items.json
    ↓ (JSON 파싱)
List<Dictionary<string, object>>
    ↓ (프로필 로드)
ItemProfile (필드 정의)
    ↓ (ScriptableObject 생성)
DynamicItemDataSO (동적 속성)
    ↓ (저장)
Assets/Resources/RundeeItemFactory/[Type]Items/
```

---

## 사용 시나리오

### 시나리오 1: 새로운 아이템 타입 생성

1. **Item Profile 생성**
   - Unity Editor에서 `Tools > Rundee > Item Factory` 열기
   - "Item Profile Manager" 탭으로 이동
   - "Create New Profile" 클릭
   - 필드 추가 (예: `magicPower`, `elementType`, `spellLevel`)
   - World Context 설정 (예: "Generate magical items for a fantasy RPG")
   - 저장

2. **Player Profile 생성/선택**
   - "Player Profile Manager" 탭으로 이동
   - 기존 프로필 선택 또는 새로 생성

3. **아이템 생성**
   - "Item Factory" 탭으로 이동
   - 생성한 Item Profile 선택
   - Player Profile 선택
   - 모델 선택 (예: `llama3`)
   - 아이템 수 설정 (예: 20)
   - "Generate Items" 클릭

4. **Import**
   - 생성 완료 후 "Import Item From Json" 탭으로 이동
   - 생성된 JSON 파일 선택
   - "Import Selected JSON File" 클릭

### 시나리오 2: 기존 아이템 타입 확장

1. **Item Profile 편집**
   - "Item Profile Manager"에서 기존 프로필 선택
   - 새 필드 추가 (예: `durability`, `repairCost`)
   - Validation 규칙 설정
   - 저장

2. **재생성**
   - 기존 JSON 파일은 그대로 유지
   - 새로 생성하면 기존 아이템과 병합 (중복 ID 제외)

### 시나리오 3: 배치 생성

1. **여러 프로필 준비**
   - 다양한 Item Profile 생성 (예: `weapons`, `armor`, `consumables`)

2. **순차 생성**
   - 각 프로필별로 아이템 생성
   - 모든 JSON 파일이 `ItemJson/`에 저장

3. **일괄 Import**
   - "Import Item From Json" 탭에서 여러 파일 선택
   - 각각 Import 실행

---

## 기술 스택

### C++ 프로젝트

- **언어**: C++17
- **빌드 시스템**: MSBuild (Visual Studio)
- **JSON 라이브러리**: nlohmann/json
- **HTTP 클라이언트**: Windows HTTP Services (WinHTTP)
- **플랫폼**: Windows 10/11 (64-bit)

### Unity 프로젝트

- **Unity 버전**: 2020.3 이상
- **언어**: C#
- **주요 API**:
  - `EditorWindow`: 커스텀 에디터 윈도우
  - `ScriptableObject`: 런타임 데이터 저장
  - `UnityWebRequest`: HTTP 통신
  - `AssetDatabase`: 에셋 관리
  - `EditorApplication.update`: 코루틴 대체

### 외부 의존성

- **Ollama**: 로컬 LLM 서버
  - API: HTTP REST
  - 기본 포트: 11434
  - 지원 모델: llama3, mistral, gemma 등

### 프로젝트 구조 특징

1. **모듈화**: 각 컴포넌트가 명확히 분리됨
2. **확장성**: 프로필 시스템으로 코드 수정 없이 확장 가능
3. **타입 안전성**: C++의 강한 타입 시스템 활용
4. **에러 처리**: 중앙화된 에러 처리 시스템
5. **사용자 경험**: Unity Editor 통합으로 직관적인 UI 제공

---

## 결론

RundeeItemFactory는 LLM을 활용한 게임 아이템 생성 시스템으로, 다음과 같은 강점을 가집니다:

1. **유연성**: Dynamic Profile System으로 어떤 아이템 타입도 정의 가능
2. **자동화**: 프로필 생성부터 ScriptableObject 변환까지 자동화
3. **오프라인 작동**: 로컬 LLM으로 완전 오프라인 환경에서 작동
4. **Unity 통합**: Unity Editor와 완전히 통합된 워크플로우
5. **확장성**: 코드 수정 없이 새로운 아이템 타입 추가 가능

이 시스템은 게임 개발자가 대량의 아이템 데이터를 효율적으로 생성하고 관리할 수 있도록 지원합니다.

