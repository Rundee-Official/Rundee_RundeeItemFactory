# RundeeItemFactory 프로젝트 검토 보고서

## 📊 현재 진행 상황

### ✅ 완료된 항목 (Phase 0-1 기준)

#### Phase 0: 준비 & LLM 입문 ✅ **100% 완료**
- [x] 로컬 LLM 환경 구축 (Ollama)
- [x] LLM을 코드에서 호출하는 최소 예제 (C++ `OllamaClient`)
- [x] JSON 포맷으로 출력시키는 연습
- [x] Food/Material 아이템 JSON 배열 생성 및 파싱

#### Phase 1: Basic 버전 개발 ✅ **약 85% 완료**
- [x] 코어 도메인 설계 (Food/Material 스키마)
- [x] Basic 프롬프트 템플릿 설계 (프리셋 시스템 포함)
- [x] Basic CLI/스크립트 툴 (C++ 기반)
- [x] Unity 연동 - Food 버전 완료
- [x] Unity 연동 - Material 버전 **방금 추가 완료** ✨
- [ ] Unreal 연동 - 최소 버전 (아직 미구현)

---

## 🎯 코드 품질 평가

### 강점 (Strengths)

1. **깔끔한 아키텍처**
   - 명확한 책임 분리 (Parser, Writer, Generator, Client)
   - C++ 기반으로 성능과 확장성 확보
   - Unity 통합이 깔끔하게 분리됨

2. **프리셋 시스템**
   - Forest/Desert/Coast/City 프리셋으로 테마별 아이템 생성
   - `GetPresetFlavorText()` 함수로 세계관 컨텍스트 제공
   - 확장 가능한 구조

3. **검증 로직**
   - `ValidateMaterialItem()` - 수치 범위 클램핑
   - `CleanJsonArrayText()` - LLM 응답 정리
   - 안전한 JSON 파싱 (`GetIntSafe`, `GetStringSafe`)

4. **에러 처리**
   - 파일 읽기/쓰기 실패 처리
   - JSON 파싱 예외 처리
   - 빈 응답 체크

5. **개발자 경험**
   - 명령줄 인자 파싱 (`--mode`, `--preset`, `--itemType` 등)
   - 상세한 로그 출력
   - `.raw.json` 파일로 LLM 원본 응답 보존

### 개선 가능한 부분

1. **LLM 호출 방식**
   - 현재: `_popen`으로 `ollama run` 명령 실행
   - 개선: HTTP API 직접 호출 (`http://localhost:11434/api/generate`)
     - 더 빠르고 안정적
     - 스트리밍 지원 가능
     - 에러 처리 개선

2. **에러 복구**
   - LLM 응답이 유효하지 않을 때 재시도 로직 없음
   - 부분 실패 시 성공한 아이템만 저장하는 옵션 없음

3. **밸런스 검증**
   - 생성된 아이템의 통계 확인 기능 없음
   - 레어리티별 평균 수치 확인 불가
   - 밸런스 리포트 생성 기능 없음

4. **배치 생성**
   - 대량 생성 시 (50개 이상) 안정성 테스트 필요
   - LLM 타임아웃 설정 없음

---

## 📈 생성된 JSON 품질 평가

### Food JSON (`foods_city.json`)
✅ **우수한 점:**
- 스키마 일관성: 모든 필드가 올바르게 채워짐
- 밸런스: hungerRestore/thirstRestore 값이 합리적
- 다양성: 스낵/라이트/드링크 카테고리 구분 명확
- 설명: 각 아이템 설명이 적절함

⚠️ **개선 가능:**
- 일부 아이템이 `category: "Drink"`인데 Food 프롬프트로 생성됨 (의도된 것일 수 있음)
- `spoilTimeMinutes` 값이 일관적이지 않음 (30/60분 혼재)

### Material JSON (`materials_city.json`)
✅ **우수한 점:**
- 카테고리 다양성: Material/Junk/Component 구분 명확
- MaterialType 다양성: Wood/Metal/Plastic/Electronic 등
- 밸런스: 레어리티에 따른 value 분포가 합리적
- 설명: 각 아이템의 용도가 명확히 설명됨

⚠️ **개선 가능:**
- `maxStack` 값이 1인 아이템이 있음 (Scrap Metal, Electronic Board) - 의도된 것일 수 있음
- `flammability` 값이 0인 아이템 (Electronic Board) - 합리적이지만 검증 필요

---

## 🚀 다음 단계 제안 (우선순위 순)

### 1. **즉시 개선 가능** (1-2일)

#### A. LLM HTTP API 직접 호출
```cpp
// 현재: _popen("ollama run ...")
// 개선: HTTP POST to http://localhost:11434/api/generate
```
- 장점: 더 빠르고 안정적, 스트리밍 지원
- 구현: `OllamaClient.cpp` 수정, HTTP 라이브러리 추가 (예: libcurl 또는 Windows HTTP API)

#### B. 밸런스 리포트 기능
```cpp
void PrintBalanceReport(const std::vector<ItemFoodData>& items);
void PrintBalanceReport(const std::vector<ItemMaterialData>& items);
```
- 평균/최소/최대 수치 출력
- 레어리티별 통계
- 밸런스 이상 감지

#### C. 재시도 로직
```cpp
std::string OllamaClient::RunWithRetry(const std::string& modelName, 
                                        const std::string& prompt, 
                                        int maxRetries = 3);
```

### 2. **Phase 2 준비** (1-2주)

#### A. Unity EditorWindow 통합
- GUI로 모델 선택, 프리셋 선택, 파라미터 입력
- "Generate" 버튼으로 한 번에 실행
- 생성 로그 표시

#### B. 배치 생성 안정화
- 50-200개 아이템 생성 테스트
- 부분 실패 시 성공한 아이템만 저장
- 진행률 표시

#### C. 아이템 관리 기능
- 이미 생성된 아이템 목록 표시
- 필터 (카테고리/레어리티/테마)
- 선택한 아이템만 재생성/삭제

### 3. **Phase 3 연구** (장기)

- 게임 난이도 기반 자동 밸런싱
- 데이터 분석 시각화
- CI/CD 통합

---

## 💡 코드 개선 예시

### 예시 1: HTTP API 직접 호출 (개선안)

```cpp
// OllamaClient.h
class OllamaClient
{
public:
    static std::string RunSimple(const std::string& modelName, 
                                  const std::string& prompt);
    
    // 개선: HTTP API 직접 호출
    static std::string RunViaHttp(const std::string& modelName, 
                                   const std::string& prompt,
                                   int timeoutSeconds = 60);
};
```

### 예시 2: 밸런스 리포트 (추가 기능)

```cpp
// BalanceReporter.h
class BalanceReporter
{
public:
    static void PrintFoodBalanceReport(const std::vector<ItemFoodData>& items);
    static void PrintMaterialBalanceReport(const std::vector<ItemMaterialData>& items);
};
```

---

## 📝 결론

### 현재 상태: **Phase 1 거의 완료** ✅

**강점:**
- 핵심 기능이 모두 동작함
- 코드 품질이 우수함
- 확장 가능한 아키텍처
- Unity 통합이 잘 되어 있음

**다음 단계:**
1. Material Unity 통합 완료 (방금 추가됨) ✨
2. LLM HTTP API 직접 호출로 개선
3. 밸런스 리포트 기능 추가
4. Unity EditorWindow 통합 (Phase 2)

**전체 평가: 8.5/10** 🌟

프로젝트가 매우 잘 진행되고 있습니다! Phase 1의 목표를 거의 달성했고, Phase 2로 넘어갈 준비가 되어 있습니다.

---

## 🎯 체크리스트

### Phase 1 완료 체크
- [x] 로컬 LLM 연동
- [x] Food JSON 생성
- [x] Material JSON 생성
- [x] Unity Food 임포트
- [x] Unity Material 임포트 (방금 추가)
- [ ] Unreal 연동 (선택사항)
- [ ] 엔드 투 엔드 테스트 문서화

### Phase 2 준비 체크
- [ ] Unity EditorWindow UI
- [ ] 배치 생성 안정화
- [ ] 아이템 관리 기능
- [ ] 밸런스 리포트




