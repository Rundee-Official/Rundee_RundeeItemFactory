# 코드 구조 문서

## 파일 구조

### C++ 프로젝트 구조

```
RundeeItemFactory/
├── RundeeItemFactory/
│   ├── include/              # 헤더 파일
│   │   ├── Data/            # 아이템 데이터 구조
│   │   ├── Validators/      # 검증 로직
│   │   ├── Parsers/         # JSON 파싱
│   │   ├── Writers/          # JSON 작성
│   │   ├── Generators/      # 아이템 생성
│   │   ├── Clients/         # LLM 클라이언트 (Ollama)
│   │   ├── Prompts/         # 프롬프트 빌더
│   │   ├── Helpers/         # 유틸리티
│   │   └── Utils/           # 공통 유틸리티
│   └── src/                 # 구현 파일
│       ├── Generators/
│       │   └── ItemGenerator.cpp  # 메인 생성 로직 (2788 lines)
│       ├── Parsers/
│       ├── Writers/
│       ├── Clients/
│       ├── Prompts/
│       └── Helpers/
```

### Unity 프로젝트 구조

```
UnityRundeeItemFactory/
└── Assets/
    └── RundeeItemFactory/
        ├── Editor/          # Unity Editor 스크립트
        │   ├── ItemFactoryWindow.cs          # 메인 생성 창
        │   ├── ItemImporter.cs              # JSON 임포트
        │   ├── ItemManagerWindow.cs         # 아이템 관리
        │   ├── RegistryManagerWindow.cs     # 레지스트리 관리 (신규)
        │   ├── ItemPreviewWindow.cs         # 아이템 미리보기 (신규)
        │   ├── BatchJobManager.cs           # 배치 작업 관리 (신규)
        │   ├── StatisticsDashboardWindow.cs # 통계 대시보드 (신규)
        │   ├── VersionManager.cs            # 버전 관리 (신규)
        │   ├── HelpWindow.cs                # 도움말 (신규)
        │   ├── PresetManagerWindow.cs       # 프리셋 관리 (신규)
        │   ├── QualityCheckWindow.cs         # 품질 검사 (신규)
        │   ├── DataExporter.cs              # 데이터 내보내기 (신규)
        │   └── ErrorHandler.cs              # 에러 처리 (신규)
        └── Runtime/          # 런타임 스크립트
            ├── FoodItemDataSO.cs
            ├── DrinkItemDataSO.cs
            ├── MaterialItemDataSO.cs
            ├── WeaponItemDataSO.cs
            ├── WeaponComponentItemDataSO.cs
            ├── AmmoItemDataSO.cs
            └── ItemDatabase.cs
```

## ItemGenerator.cpp 구조

### 1. 파일 헤더 (Lines 1-18)
- Doxygen 파일 주석
- 프로젝트 정보 및 주요 기능 설명

### 2. Includes (Lines 20-48)
- 필요한 헤더 파일들
- 표준 라이브러리 포함

### 3. Anonymous Namespace - 헬퍼 함수들 (Lines 50-152)
- `kBannedWords`: 금지 단어 목록
- `kRegistryDir`: 레지스트리 디렉토리
- `fileMutexes`: 파일별 뮤텍스 관리
- `GetFileMutex()`: 파일 뮤텍스 가져오기
- `ToLower()`: 문자열 소문자 변환
- `CountBanHits()`: 금지 단어 카운트
- `CountRarity()`: 레어도 분포 계산
- `PrintGuardrailSummary()`: 가드레일 요약 출력

### 4. 유틸리티 함수들 (Lines 154-334)
- `EnsureParentDir()`: 부모 디렉토리 생성
- `GetRegistryPath()`: 레지스트리 파일 경로
- `LoadRegistryIds()`: 레지스트리 ID 로드
- `SaveRegistryIds()`: 레지스트리 ID 저장
- `LogRegistryEvent()`: 레지스트리 이벤트 로깅
- `AppendIdsToRegistry()`: 레지스트리에 ID 추가
- `CleanJsonTrailingCommas()`: JSON 후행 쉼표 제거
- `SaveTextFile()`: 텍스트 파일 저장
- `GetCurrentTimestamp()`: 현재 타임스탬프

### 5. Forward Declarations (Lines 336-367)
- 모든 `ProcessLLMResponse_*` 함수 선언
- `ItemGenerator::GenerateWithLLM_SingleBatch` 선언

### 6. Common Template Function (Lines 369-763)
- `ProcessLLMResponse_Common<>`: 공통 템플릿 함수
- 모든 아이템 타입에서 공통으로 사용되는 로직

### 7. Type-Specific Implementations (Lines 765-1818)
각 타입별로 `ProcessLLMResponse_Common`을 래핑:
- `ProcessLLMResponse_Food` (Line 765)
- `ProcessLLMResponse_Drink` (Line 800)
- `ProcessLLMResponse_Material` (Line 835)
- `ProcessLLMResponse_Weapon` (Line 866)
- `ProcessLLMResponse_WeaponComponent` (Line 905)
- `ProcessLLMResponse_Armor` (Line 1319)
- `ProcessLLMResponse_Clothing` (Line 1362)
- `ProcessLLMResponse_Ammo` (Line 1405)

### 8. Main ItemGenerator Namespace (Lines 1820-2788)
- `ItemGenerator::GenerateWithLLM()`: 메인 생성 함수
- 병렬 배치 처리 로직
- 최종 병합 및 검증

## 함수 위치 가이드라인

### ✅ 올바른 위치

1. **헬퍼 함수**: Anonymous namespace 또는 파일 상단
2. **타입별 처리 함수**: 타입별 섹션에 그룹화
3. **메인 로직**: `ItemGenerator` namespace 내부
4. **템플릿 함수**: 타입별 구현 전에 배치

### 📝 섹션 구분

```cpp
// ============================================================================
// SECTION: [섹션 이름]
// ============================================================================
// 설명
```

현재 섹션:
- `SECTION: ID Registry Management`
- `SECTION: Forward Declarations`
- `SECTION: Common Template-Based Processing Logic`
- `SECTION: Type-Specific ProcessLLMResponse Implementations`
- `SECTION: Main ItemGenerator Namespace`

## Doxygen 주석 규칙

### 파일 헤더
```cpp
/**
 * @file FileName.cpp
 * @brief 간단한 설명
 * @author 작성자
 * @date 날짜
 * @copyright 저작권 정보
 * 
 * 상세 설명...
 */
```

### 함수 주석
```cpp
/**
 * @brief 함수의 간단한 설명
 * @param paramName 매개변수 설명
 * @return 반환값 설명
 * @details 상세 설명 (필요시)
 * @note 참고사항 (필요시)
 */
```

### 클래스/구조체 주석
```cpp
/**
 * @class ClassName
 * @brief 클래스 설명
 * 
 * 상세 설명...
 */
```

## Unity 파일 구조

### Editor 스크립트
- **위치**: `Assets/RundeeItemFactory/Editor/`
- **용도**: Unity Editor 전용 기능
- **특징**: `#if UNITY_EDITOR` 불필요 (Editor 폴더는 자동으로 Editor 전용)

### Runtime 스크립트
- **위치**: `Assets/RundeeItemFactory/Runtime/`
- **용도**: 게임 실행 시 사용되는 스크립트
- **특징**: ScriptableObject, MonoBehaviour 등

### 파일 명명 규칙
- **Window 클래스**: `*Window.cs` (예: `ItemFactoryWindow.cs`)
- **Manager 클래스**: `*Manager.cs` (예: `BatchJobManager.cs`)
- **Utility 클래스**: `*Handler.cs`, `*Exporter.cs` 등 (예: `ErrorHandler.cs`)
- **Data 클래스**: `*DataSO.cs` (예: `FoodItemDataSO.cs`)

## 개선 사항

### 완료된 개선
- ✅ Doxygen 주석 추가
- ✅ 섹션 구분 명확화
- ✅ 함수별 주석 추가
- ✅ 파일 구조 정리

### 권장 사항
1. **함수 길이**: 200줄 이하로 유지 (현재 일부 함수가 길음)
2. **순환 복잡도**: 복잡한 함수는 더 작은 함수로 분리
3. **네이밍**: 일관된 네이밍 컨벤션 유지
4. **주석**: 복잡한 로직에 인라인 주석 추가



