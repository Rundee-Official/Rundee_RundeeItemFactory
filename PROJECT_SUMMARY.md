# RundeeItemFactory 프로젝트 요약

로컬 LLM(Ollama)을 사용해 게임 아이템 데이터를 생성하고, Unity ScriptableObject로 자동 변환하는 툴체인입니다. 모든 아이템 타입(Food, Drink, Material, Weapon, WeaponComponent, Ammo)을 지원하며, 단일 Unity 메뉴 흐름으로 생성 → 임포트 → 관리가 가능합니다.

---

## 주요 기능
- **LLM 기반 아이템 생성**: Ollama + 프롬프트 템플릿 시스템
- **여러 아이템 타입 지원**: Food/Drink/Material/Weapon/WeaponComponent/Ammo
- **프리셋 시스템**: Default, Forest, Desert, Coast, City + 커스텀 프리셋
- **Unity 통합**:
  - Item Factory Window: 생성·자동 임포트·밸런스 리포트
  - **JSON Importer**: 모든 타입을 한 창에서 수동 임포트 (기존 per-type 메뉴 제거)
  - Item Manager: 생성된 ScriptableObject 검색/정리
- **밸런스 리포트**: 통계/분포/경고 출력
- **배치 처리**: 여러 타입을 한 번에 생성, 부분 실패 허용

---

## 최근 업데이트
- Unity 메뉴 정리: 수동 임포트가 `Tools > Rundee > JSON Importer` 하나로 통합
- 빌드 설정: vcxproj에 **C++17** 명시, `std::filesystem` 기반 AppConfig 경로 처리 안정화
- AppConfig: `config/rundee_config.json` 기본 경로 로딩 및 예외 처리 개선
- 문서: README, E2E, 배치 계획, 사용 예제 등을 최신 메뉴/흐름에 맞게 갱신

---

## 기술 스택
- **C++17** (Visual Studio), nlohmann/json, Windows API
- **Unity 2020.3+**, C# Editor 확장, ScriptableObject/Resources
- **LLM**: Ollama (llama3 계열 등), 프롬프트 템플릿 기반

---

## 프로젝트 구조 (요약)
```
RundeeItemFactory/          # C++ 생성기
├── include/ ...            # Data, Validators, Parsers, Writers, Prompts, Helpers, Utils
└── src/ ...                # 구현

UnityRundeeItemFactory/     # Unity 프로젝트
└── Assets/RundeeItemFactory/
    ├── Editor/             # ItemFactoryWindow, JSON Importer, ItemManager 등
    └── Runtime/            # ScriptableObject 및 런타임 컴포넌트
```

---

## 사용 흐름 (요약)
1. C++ 빌드: `RundeeItemFactory.exe` 생성 (Release/Debug)
2. Item Factory Window에서 아이템 생성(+자동 임포트) 또는 CLI로 JSON 생성
3. **JSON Importer**에서 타입 선택 후 JSON 파일 수동 임포트
4. Item Manager로 생성 자산 확인/정리

자세한 절차는 `README.md`, `USAGE_EXAMPLES.md`, `docs/E2E_TEST_PLAN.md`를 참고하세요.

---

## 앞으로의 권장 작업
- JSON Importer/E2E 스모크 테스트 재실행 후 결과 기록
- 간단한 CI 초안: 빌드 + 파서/밸런스 리포트 단위 테스트
- HTTP API(ollama serve) 경로 프로토타입 후 성능/안정성 비교
- 배치 스트레스 플랜 재검증 및 로그 보관


