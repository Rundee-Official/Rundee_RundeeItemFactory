# 리팩토링 요약

## 완료된 작업

### 1. Utils 파일 생성 ✅
- **StringUtils.h/cpp**: 문자열 유틸리티 함수들
  - `TrimString()`: 공백 제거
  - `CleanJsonArrayText()`: LLM 응답 JSON 정리
  - `EscapeForCmd()`: 명령줄 이스케이프
  - `EscapeString()`: JSON 이스케이프
  - `NormalizeWhitespace()`: 공백 정규화

- **JsonUtils.h/cpp**: JSON 파싱 유틸리티
  - `GetIntSafe()`: 안전한 정수 파싱
  - `GetBoolSafe()`: 안전한 불린 파싱
  - `GetStringSafe()`: 안전한 문자열 파싱
  - `ClampInt()`: 정수 범위 제한

### 2. OOP 구조 개선 ✅
- **ItemDataBase.h**: 모든 아이템 타입의 기본 클래스
  - 공통 필드: `id`, `displayName`, `category`, `rarity`, `maxStack`, `description`
  - 가상 메서드: `GetItemTypeName()`

- **ItemFoodData**: `ItemDataBase` 상속
  - Food 전용 필드: `hungerRestore`, `thirstRestore`, `healthRestore`, `spoils`, `spoilTimeMinutes`

- **ItemMaterialData**: `ItemDataBase` 상속
  - Material 전용 필드: `materialType`, `hardness`, `flammability`, `value`

### 3. 통합 Parser/Writer ✅
- **ItemJsonParser.h/cpp**: 통합 JSON 파서
  - `ParseFoodFromJsonText()`: Food 아이템 파싱
  - `ParseMaterialFromJsonText()`: Material 아이템 파싱

- **ItemJsonWriter.h/cpp**: 통합 JSON 작성기
  - `WriteFoodToFile()`: Food 아이템 저장
  - `WriteMaterialToFile()`: Material 아이템 저장

### 4. 프롬프트 빌더 분리 ✅
- **PromptBuilder.h/cpp**: 프롬프트 생성 로직 분리
  - `GetPresetFlavorText()`: 프리셋별 세계관 텍스트
  - `BuildFoodJsonPrompt()`: Food 프롬프트 생성
  - `BuildMaterialJsonPrompt()`: Material 프롬프트 생성

### 5. Validator 통합 ✅
- **FoodItemValidator**: 기존 유지, `JsonUtils::ClampInt()` 사용
- **MaterialItemValidator**: 새로 생성, `JsonUtils::ClampInt()` 사용

### 6. 메인 파일 업데이트 ✅
- **RundeeItemFactory.cpp**: 새 구조 사용
  - `PromptBuilder` 사용
  - `ItemJsonParser` 사용
  - `ItemJsonWriter` 사용

### 7. 불필요한 파일 삭제 ✅
삭제된 파일:
- `FoodJsonParser.h/cpp`
- `FoodJsonWriter.h/cpp`
- `MaterialJsonParser.h/cpp`
- `MaterialJsonWriter.h/cpp`

## 새로운 파일 구조

```
include/
├── ItemDataBase.h          (새로 생성)
├── ItemFoodData.h          (수정: 상속 구조)
├── ItemMaterialData.h     (수정: 상속 구조)
├── ItemJsonParser.h        (새로 생성)
├── ItemJsonWriter.h        (새로 생성)
├── PromptBuilder.h         (새로 생성)
├── Utils/
│   ├── StringUtils.h       (새로 생성)
│   └── JsonUtils.h         (새로 생성)
├── FoodItemValidator.h      (기존 유지)
├── MaterialItemValidator.h (새로 생성)
└── ...

src/
├── ItemJsonParser.cpp      (새로 생성)
├── ItemJsonWriter.cpp      (새로 생성)
├── PromptBuilder.cpp       (새로 생성)
├── Utils/
│   ├── StringUtils.cpp     (새로 생성)
│   └── JsonUtils.cpp       (새로 생성)
├── FoodItemValidator.cpp   (수정: JsonUtils 사용)
├── MaterialItemValidator.cpp (새로 생성)
├── OllamaClient.cpp        (수정: StringUtils 사용)
└── RundeeItemFactory.cpp   (수정: 새 구조 사용)
```

## 개선 효과

1. **코드 재사용성 향상**: static 함수들을 Utils로 모아서 재사용 가능
2. **OOP 구조**: Food와 Material이 공통 기본 클래스 상속으로 일관성 확보
3. **관심사 분리**: 프롬프트 빌딩 로직이 별도 파일로 분리
4. **유지보수성 향상**: 통합된 Parser/Writer로 중복 코드 제거
5. **확장성**: 새로운 아이템 타입 추가 시 기본 클래스 상속만 하면 됨

## 다음 단계 제안

1. **템플릿 기반 Parser/Writer**: 현재는 Food/Material 각각 메서드가 있지만, 템플릿으로 더 통합 가능
2. **ItemGenerator 통합**: Food와 Material Generator도 기본 클래스로 통합 가능
3. **설정 파일**: 프롬프트 템플릿을 외부 파일로 분리




