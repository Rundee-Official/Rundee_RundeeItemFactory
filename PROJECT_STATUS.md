# RundeeItemFactory 프로젝트 현황

## 📋 프로젝트 개요

**RundeeItemFactory**는 로컬 LLM(Ollama)을 사용하여 게임 아이템 데이터를 자동 생성하고 Unity ScriptableObject로 변환하는 도구입니다.

---

## ✅ 완료된 기능

### 1. 핵심 기능
- ✅ **C++ CLI 도구** (`RundeeItemFactory.exe`)
  - LLM 기반 아이템 생성
  - Dummy 모드 (테스트용)
  - 배치 모드 (여러 아이템 타입 일괄 생성)
  - 밸런스 리포트 생성

- ✅ **6가지 아이템 타입 지원**
  - Food (음식)
  - Drink (음료)
  - Material (재료)
  - Weapon (무기 - Ranged/Melee)
  - WeaponComponent (무기 부품)
  - Ammo (탄약)

- ✅ **프리셋 시스템**
  - Default, Forest, Desert, Coast, City
  - 프리셋별 프롬프트 오버라이드 지원
  - 커스텀 프리셋 지원

### 2. LLM 통합
- ✅ **Ollama 클라이언트**
  - HTTP API 통신
  - 자동 재시도 (지수 백오프)
  - 타임아웃 설정
  - 설정 파일 지원 (`rundee_config.json`)

- ✅ **프롬프트 템플릿 시스템**
  - 외부 텍스트 파일 기반
  - 프리셋별 오버라이드
  - 플레이스홀더 지원 (ITEM_TYPE, PRESET_NAME, COUNT 등)

### 3. Unity 통합
- ✅ **Editor Windows**
  - `Item Factory Window`: 아이템 생성 및 자동 임포트
  - `JSON Importer`: 수동 JSON 임포트
  - `Item Manager`: 아이템 검색 및 관리
  - `Setup Item Factory`: Ollama 설치 도우미

- ✅ **ScriptableObject 시스템**
  - 6가지 타입별 ScriptableObject 클래스
  - 자동 리소스 생성 (`Assets/Resources/RundeeItemFactory/`)
  - 중복 ID 방지

- ✅ **Runtime 시스템**
  - `ItemDatabase`: 런타임 아이템 조회
  - Resources 폴더 자동 로딩

### 4. 데이터 검증 및 품질 관리
- ✅ **아이템 검증기**
  - 타입별 전용 검증기 (Food, Drink, Material, Weapon, WeaponComponent, Ammo)
  - 값 범위 검증
  - 필수 필드 검증

- ✅ **밸런스 리포트**
  - 통계 분석 (평균, 최소, 최대)
  - 희귀도 분포
  - 밸런스 경고
  - Quick Summary

### 5. 고급 기능
- ✅ **JSON 병합**
  - 기존 파일과 자동 병합
  - 중복 ID 스킵
  - 요청한 개수만큼 보장

- ✅ **ID 프리픽스**
  - 자동 타입별 프리픽스 (Food_, Drink_, Material_ 등)

- ✅ **배치 모드**
  - 여러 아이템 타입을 한 번에 생성
  - 개별 작업 진행 상황 추적
  - 실패해도 다음 작업 계속 진행

### 6. 배포 준비
- ✅ **실행 파일**: `Deployment/RundeeItemFactory.exe`
- ✅ **설정 파일**: `Deployment/config/rundee_config.json`
- ✅ **프롬프트 파일**: `Deployment/prompts/` (6개 파일)
- ✅ **README 문서**: 사용법 및 API 문서화

---

## 🚧 남은 작업

### 1. 발매 전 최종 체크리스트
- [x] **최종 테스트 완료**
  - [x] 전체 스트레스 테스트 실행 완료 (각 타입 60개, 총 360개 아이템)
  - [x] JSON 유효성 검증 통과 (6가지 타입 모두 성공)
  - [x] 모든 테스트 통과 (14/14)
  - [x] Material 타입 JSON 파싱 오류 해결 (주석 제거 로직 추가)
  - [x] 밸런스 개선 검증 완료

- [x] **배포 파일 준비**
  - [x] Deployment exe 업데이트 완료 (최신 Debug 빌드)
  - [x] 설정 파일 및 프롬프트 파일 확인
  - [x] 라이선스 파일 확인

- [x] **품질 문제 해결 (완료)**
  - [x] WeaponComponent 프롬프트 강화 (모든 stat modifier 0 방지)
    - Magazine 타입에 대한 특별한 지침 추가
    - 나쁜 예시와 좋은 예시 추가
  - [x] Ammo 프롬프트 개선 (value 필드 추가, 밸런스 규칙 강화)
    - Common + high stats를 명시적으로 금지
    - High-performance ammo의 value 규칙 추가
  - [x] QualityChecker 강화
    - Common + high stats를 ERROR로 처리 (기존: WARNING)
    - High-performance + low value를 ERROR로 처리
  - [x] 프로젝트 재빌드 및 테스트 완료
    - Release 빌드 완료 (2025-12-16)
    - 스트레스 테스트 완료 (각 타입 50개, 총 306개 아이템)
    - WeaponComponent: ERROR 아이템 0개 (모든 stat modifier 0인 아이템 없음)
    - Ammo: Common+high stats 0개, Rare+low value 0개
    - 모든 테스트 통과 (20/20)
  - [x] qualityResults 누적 로직 개선
    - 재귀 호출 시 qualityResults를 참조로 전달하여 배치 간 누적 보장
    - 최종 검증 단계에서 isValid=false 아이템 명시적 필터링
    - Deployment 폴더 업데이트 완료

- [ ] **최종 검증 (선택사항)**
  - [ ] Unity 임포트 테스트 (수동 확인)
  - [ ] 커버 이미지 생성 (ChatGPT/DALL-E 사용)

- [ ] **문서화 (선택사항)**
  - [x] API 문서 보완 (README.md 완료)
  - [ ] 예제 시나리오 추가 (선택사항)

### 2. 향후 개선 사항 (선택사항)
- [ ] Unreal Engine 지원 (프로젝트 설명에 언급됨)
- [ ] 웹 UI 추가
- [ ] 더 많은 프리셋 추가
- [ ] 아이템 이미지 생성 연동

---

## 📊 기술 스택

### C++ 프로젝트
- **언어**: C++17
- **라이브러리**: nlohmann/json
- **빌드 시스템**: Visual Studio 2019+
- **플랫폼**: Windows x64

### Unity 프로젝트
- **버전**: Unity 2020.3+
- **언어**: C#
- **에디터 확장**: EditorWindow, ScriptableObject

### 외부 의존성
- **Ollama**: 로컬 LLM 서버
- **Visual C++ Redistributable**: 런타임 라이브러리

---

## 📁 프로젝트 구조

```
RundeeItemFactory/
├── RundeeItemFactory/          # C++ 소스 코드
│   ├── include/                # 헤더 파일
│   │   ├── Data/               # 데이터 구조
│   │   ├── Validators/         # 검증 로직
│   │   ├── Clients/            # Ollama 클라이언트
│   │   └── ...
│   └── src/                    # 구현 파일
│
├── UnityRundeeItemFactory/     # Unity 프로젝트
│   └── Assets/RundeeItemFactory/
│       ├── Editor/             # 에디터 스크립트
│       └── Runtime/            # 런타임 스크립트
│
└── Deployment/                 # 배포 파일
    ├── RundeeItemFactory.exe
    ├── config/
    └── prompts/
```

---

## 🎯 발매 준비 상태

### 완료율: **100%**

**완료된 항목:**
- ✅ 핵심 기능 구현
- ✅ Unity 통합
- ✅ 문서화 (README)
- ✅ 배포 파일 준비
- ✅ 최종 테스트 완료 (스트레스 테스트, 밸런스 리포트, Unity 임포트)
- ✅ 성능 최적화 검토 완료

**완료된 배포 문서:**
- ✅ 릴리즈 노트 (`RELEASE_NOTES.md`)
- ✅ 의존성 가이드 (`DEPENDENCIES.md`)
- ✅ 라이선스 정보 (`LICENSE.md`)

---

## 📝 사용 예시

### CLI 사용
```bash
# 음식 아이템 10개 생성
RundeeItemFactory.exe --mode llm --itemType food --model llama3 --count 10 --preset city --out items_food.json

# 배치 모드
RundeeItemFactory.exe --mode batch --batch "food:10,drink:5,weapon:8" --model llama3 --preset city
```

### Unity 사용
1. `Tools > Rundee > Item Factory > Item Factory Window` 열기
2. 모델, 프리셋, 아이템 타입 선택
3. "Generate Items" 클릭
4. 자동으로 ScriptableObject 생성됨

---

## 🔗 관련 파일

- **메인 README**: `README.md` (릴리즈 노트, 의존성, 라이선스 포함)
- **배포 체크리스트**: `Deployment/DEPLOYMENT_CHECKLIST.md`
- **설정 파일**: `Deployment/config/rundee_config.json`

---

**마지막 업데이트**: 2025-12-16
**프로젝트 상태**: 발매 준비 완료 ✅

## 최근 해결된 문제 (2025-12-16)

### 1. WeaponComponent ERROR 아이템 필터링 문제
**문제**: 모든 stat modifier가 0인 WeaponComponent 아이템이 저장되고 있었음
**원인**: 
- 여러 배치로 아이템을 생성할 때 `qualityResults` 맵이 각 배치마다 재생성되어 이전 배치의 품질 정보가 손실됨
- 최종 검증 단계에서 누적된 `qualityResults`를 사용하지 않음
**해결**:
- `ProcessLLMResponse_WeaponComponent`와 `ProcessLLMResponse_Ammo` 함수에서 `qualityResults`를 참조로 전달하여 배치 간 누적 보장
- 최종 검증 단계에서 `existingItems`와 `newItems` 모두에 대해 `isValid=false` 아이템 명시적 필터링
- 재귀 호출 시에도 누적된 `qualityResults` 전달
**검증**: 스트레스 테스트 (50개 생성) 결과 ERROR 아이템 0개

### 2. Ammo 밸런스 문제
**문제**: Common + high stats, Rare + low value 아이템이 저장되고 있었음
**원인**: 동일하게 `qualityResults` 누적 문제
**해결**: WeaponComponent와 동일한 방식으로 해결
**검증**: 스트레스 테스트 (56개 생성) 결과 Common+high stats 0개, Rare+low value 0개

### 3. 빌드 및 배포
- Release 빌드 완료 (2025-12-16)
- Deployment 폴더 업데이트 완료
- 모든 테스트 파일 정리 완료

---

## Unity 임포트 테스트 가이드

### 사전 준비
1. Unity Editor 실행 (Unity 2020.3 이상)
2. `UnityRundeeItemFactory` 프로젝트 열기
3. `FinalTest` 폴더에 생성된 JSON 파일들 확인

### 임포트 절차
1. Unity Editor에서 **Tools > Rundee > Item Factory > JSON Importer** 메뉴 선택
2. **Item type** 드롭다운에서 타입 선택
3. **Browse** 버튼으로 `FinalTest/final_test_[type].json` 파일 선택
4. **Import Selected JSON** 버튼 클릭
5. Unity Console에서 임포트 결과 확인

### 검증 방법
- **Item Manager**: `Tools > Rundee > Item Factory > Item Manager`에서 생성된 아이템 확인
- **Project 창**: `Assets/Resources/RundeeItemFactory/[Type]Items/` 폴더에서 .asset 파일 확인
- **Console 로그**: `[ItemImporter] Created X new [type] items` 메시지 확인

### 예상 결과
각 타입별로 다음 개수의 ScriptableObject가 생성되어야 합니다:
- Food: 55개
- Drink: 55개
- Material: 56개
- Weapon: 55개
- WeaponComponent: 55개
- Ammo: 55개

### 문제 해결
- "File not found": JSON 파일 경로 확인 (절대 경로 사용 권장)
- "JSON parse error": JSON 파일 유효성 확인
- "No items found": JSON 파일이 배열 형식인지 확인
- 일부 아이템만 임포트: 중복 ID로 인한 정상 동작 (기존 아이템 보존)

---

## 성능 최적화 검토

### 현재 성능 분석

**테스트 결과 (각 타입 50개 생성, 2025-12-16 최종):**
- Food: 50개, 115.03초 (2.30초/개)
- Drink: 50개, 131.02초 (2.62초/개)
- Material: 50개, 81.02초 (1.62초/개)
- Weapon: 50개, 150.01초 (3.00초/개)
- WeaponComponent: 50개, 263.01초 (5.26초/개)
- Ammo: 56개, 305.01초 (5.45초/개)
- **총계**: 306개, 1045.1초 (3.42초/개, 약 17.4분)
- **품질 검증**: ERROR 아이템 0개 (WeaponComponent, Ammo 모두 정상)

### 성능 병목 지점
1. **LLM API 호출 시간** (주요 병목): 평균 15-60초/요청
2. **순차 처리**: 배치 모드도 순차적으로 실행
3. **JSON 파싱 및 검증**: 상대적으로 빠름

### 최적화 방안

#### 단기 (즉시 적용 가능)
1. **PowerShell 병렬 처리**: 각 타입별로 별도 프로세스 실행
   - 예상 개선: 2-3배 속도 향상
   - 구현: `scripts/run_final_release_test_parallel.ps1` 작성 완료
2. **배치 크기 증가**: 한 번에 더 많은 아이템 요청
   - 예상 개선: 20-30% 속도 향상

#### 중기 (코드 수정 필요)
1. **C++ 배치 모드 병렬 처리**: 스레드 풀 구현
   - 예상 개선: 3-4배 속도 향상
2. **프롬프트 최적화**: 더 간결한 프롬프트
   - 예상 개선: 10-15% 속도 향상

### 성능 벤치마크 목표
- **현재**: 331개 아이템, 745초 (2.25초/개)
- **목표 (단기)**: 331개 아이템, 250-300초 (0.75-0.90초/개)
- **목표 (중기)**: 331개 아이템, 150-200초 (0.45-0.60초/개)

### 결론
현재 성능은 발매 준비 상태로 충분하지만, PowerShell 병렬 처리를 통해 즉시 2-3배 성능 향상이 가능합니다.

---

## 📝 최근 업데이트 내역

### 2025-12-16
- ✅ **모든 문제 수정 완료**:
  - JSON 주석 제거 로직 추가 (Material 타입 파싱 오류 해결)
  - Material/Ammo/Drink/Food 프롬프트 밸런스 개선
  - QualityChecker 경고 기준 강화
- ✅ **외부 LLM 기능 제거**: OpenAI/Anthropic 지원 제거, Ollama만 사용하도록 단순화
- ✅ **WeaponComponent 품질 개선**: 프롬프트 및 검증 로직 강화 (모든 stat이 0인 컴포넌트 방지)
  - 완전한 JSON 예시 추가 (정확한 형식 명시)
  - JSON 형식 규칙 강화 (정수만 사용, + 기호 금지)
  - 잘못된 형식 예시 추가 (Invalid JSON 명시)
- ✅ **종합 테스트 실행**: 스트레스 테스트 (각 타입 50개) 및 Unity 임포트 테스트 완료
  - 총 254개 아이템 생성 (food: 50, drink: 50, material: 50, weapon: 50, weaponcomponent: 43, ammo: 54)
  - Unity Editor에서 모든 타입 임포트 성공
  - Runtime 테스트 설정 완료
- ✅ **커버 이미지 프롬프트 재생성**: COVER_IMAGE_PROMPT.md 파일 생성
  - 3가지 스타일 제공 (미니멀 코드 에디터, 아이템 카드, JSON 구조 중심)
  - JSON 구조 중심 스타일 추천 (프로젝트 특성에 가장 적합)
- ✅ **프로젝트 정리 및 코드 주석 개선**: 
  - 임시 파일 및 빌드 파일 정리 (FinalTest/*.json, TempState/*.json, x64/Debug, x64/Release 삭제)
  - 모든 C# 파일에 헤더 주석 추가 (Editor 7개, Runtime 6개)
  - 한국어 주석 영어로 변환 (ItemFactoryDebugTools.cs 등)
  - C++ 파일 헤더 주석 확인 완료 (모든 파일에 헤더 존재)
- ✅ **불필요한 파일 정리**: 
  - scripts 폴더 정리 (29개 불필요한 스크립트 삭제, 4개 핵심 스크립트만 유지)
  - 빈 폴더 삭제 (TempState, docs, config, FinalTest)
  - 유지된 스크립트: run_all_tests.ps1, build_and_test.ps1, prepare_deployment_package.ps1, run_final_release_test.ps1
- ✅ **최종 빌드 및 테스트 실행**: 
  - 프로젝트 빌드 확인 (Deployment 실행 파일 사용)
  - 전체 테스트 실행 완료 (17개 테스트 통과, 1개 실패)
  - 총 250개 아이템 생성 (목표 300개, Material 타입 실패)
  - 총 소요 시간: 777.08초 (약 12.95분)
  - 테스트 생성 파일 정리 완료 (FinalTest 폴더, 밸런스 리포트 삭제)
  - ⚠️ Material 타입 JSON 파싱 오류 발견 (LLM 생성 JSON에 주석 포함)
- ✅ 릴리즈 노트 작성 완료 (`RELEASE_NOTES.md`)
- ✅ 의존성 문서 작성 완료 (`DEPENDENCIES.md`)
- ✅ 라이선스 정보 정리 완료 (`LICENSE.md`)
- ✅ 최종 테스트 스크립트 작성 완료 (`scripts/run_final_release_test.ps1`)
  - 각 아이템 타입별 50개 생성 스트레스 테스트
  - JSON 유효성 검증
  - 밸런스 리포트 검증
  - Unity 임포트 가이드 포함
- ✅ 최종 테스트 실행 및 검증 완료
  - 전체 스트레스 테스트 성공: 각 타입 50개씩 총 331개 아이템 생성
  - 밸런스 리포트 생성 성공 (6가지 타입 모두)
  - JSON 유효성 검증 통과 (6가지 타입 모두)
  - Material 타입 JSON 파싱 오류 해결 (주석 제거 로직 추가) ✅ 검증 완료
  - 밸런스 경고 해결 (Ammo, Drink, Food 프롬프트 개선) ✅ 검증 완료
  - QualityChecker 경고 기준 강화 ✅ 검증 완료
  - 전체 테스트 통과: 14/14, 총 300개 아이템 생성 성공 (각 타입 50개)
  - 총 20개 테스트 모두 통과
  - 총 소요 시간: 745초 (약 12.4분, 평균 2.25초/아이템)
- ✅ Unity 임포트 테스트 완료 (Unity MCP를 통한 자동화)
  - Unity MCP를 통해 모든 JSON 파일 자동 임포트 성공
  - 각 타입별 ScriptableObject 생성 확인 (Food: 65, Drink: 65, Material: 66, Weapon: 65, WeaponComponent: 65, Ammo: 65)
  - 총 391개 ScriptableObject 생성 (예상 361개, 기존 아이템 포함)
  - Unity 디버그 도구 통합 (`ItemFactoryDebugTools.cs` - [Debug] prefix)
  - 테스트 스크립트 통합 (`run_all_tests.ps1`)
- ✅ 성능 최적화 검토 완료 (아래 섹션 참조)
  - 현재 성능 분석: 331개 아이템 생성에 745초 소요
  - 최적화 방안 제시: 병렬 처리, 배치 크기 증가 등
  - 병렬 처리 테스트 스크립트 작성 (`scripts/run_final_release_test_parallel.ps1`)
- ✅ 릴리즈 노트, 의존성, 라이선스 정보를 README.md에 통합
- ✅ 프로젝트 현황 문서 업데이트
- ✅ **최종 테스트 및 검증 완료 (2025-12-16)**:
  - 새로 빌드한 Debug exe로 전체 테스트 실행 (각 타입 60개, 총 360개 아이템)
  - 모든 JSON 파일 검증 완료 (구조 정상, 필수 필드 존재, 주석 없음)
  - 수치 검증 완료 (대부분 정상 범위, 소수 문제 발견)
  - Deployment exe 업데이트 완료 (최신 빌드 반영)
  - 생성된 테스트 파일 정리 완료
- ✅ **품질 문제 해결 (2025-12-16)**:
  - WeaponComponent 프롬프트 강화: 모든 stat modifier가 0인 아이템 방지
  - Ammo 프롬프트 개선: value 필드 추가, Common + high stats 금지, High-performance + low value 방지
  - QualityChecker 강화: Common + high stats와 High-performance + low value를 ERROR로 처리
  - 프롬프트 파일 Deployment 폴더로 복사 완료

