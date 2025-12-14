# 현재 프로젝트 상태

## ✅ 최근 완료/업데이트
- 모든 아이템 타입(Food/Drink/Material/Weapon/WeaponComponent/Ammo) 생성 및 Unity 임포트 경로 정비
- Unity 메뉴 통합: **Tools > Rundee > JSON Importer** 하나로 수동 임포트 (기존 per-type 메뉴 제거)
- Item Factory Window 유지(자동 임포트 지원), Item Manager 유지
- C++ 프로젝트를 **C++17**로 고정, `std::filesystem` 사용 가능하도록 vcxproj 업데이트
- AppConfig: 기본 경로(`config/rundee_config.json`) 로드 및 std::filesystem 기반 I/O 정상화

---

## 🎯 바로 다음에 하면 좋은 것들 (우선순위 제안)
1. **JSON Importer 스모크 테스트**: 각 타입 JSON 샘플로 임포트 성공/실패 로그 확인
2. **E2E 시나리오 갱신**: 새 메뉴 구조(JSON Importer) 기준으로 `docs/E2E_TEST_PLAN.md` 실행 기록 남기기
3. **자동 테스트/CI 초안**: 빌드 + 간단한 JSON 파서/밸런스 리포트 유닛 테스트 워크플로 초안 작성
4. **LLM 호출 경로 선택지 검토**: 현행 프로세스 실행 유지 vs HTTP API 전환 결정
5. **배포 체크리스트 업데이트**: Release/Debug 출력물에 `config/` + `prompts/` 복사 규칙 검증

---

## 📌 리스크 / 주의
- Unity 메뉴 변경으로 과거 문서의 "Import ... Items From JSON" 안내는 더 이상 유효하지 않음
- Ollama 미설치/미기동 시 Item Factory Window의 Setup 탭이나 별도 설치 스크립트 필요
- 대량 배치(50~200개) 시 LLM 응답 지연 가능 → 필요 시 재시도/타임아웃 설정 검토

---

## 🧭 제안 워크플로 (짧은 스프린트용)
1) JSON Importer로 단일 타입 임포트 스모크 테스트 (Food/Weapon 중 1개)  
2) E2E 문서 실행/수정 → 성공 로그 캡쳐  
3) README/NEXT_STEPS 재검토 후 CI 초안 작성  
4) 필요 시 HTTP API 전환 여부 결정 (성능/안정성 비교 메모 포함)



