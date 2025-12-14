# 다음 단계 제안

## 📊 현재 스냅샷
- Unity 수동 임포트가 **JSON Importer** 단일 창으로 통합됨
- C++17 강제 및 `std::filesystem` 정상 동작
- Item Factory Window/Item Manager는 그대로 사용 가능 (자동 임포트 옵션 포함)

---

## 🔥 바로 할 수 있는 것들 (1~2일)
1. **LLM 스모크 & 임포트**  
   - `pwsh scripts/run_smoke_tests.ps1` (6타입 LLM + 배치) 실행, 로그 캡처  
   - 생성된 JSON을 Unity JSON Importer로 임포트, 성공/에러 로그 확인
2. **파일럿 QA 준비**  
   - `docs/QA_PILOT_CHECKLIST.md` 따라 50~100개/타입 생성·임포트·QA 계획 확정  
   - 금지어/가드레일 확인 (`docs/QA_GUARDRAILS.md`)
3. **배포/설치 가이드 마무리**  
   - `docs/INSTALL_RELEASE.md` 확인: Ollama+모델+프롬프트 설치 흐름 점검

---

## 🚀 단기 (이번 주 안)
1. **CI/검증 자동화**  
   - 빌드 후 `run_smoke_tests.ps1` 실행 (옵션: -SkipBalance/-SkipBatch)  
   - `.cursor/debug.log` 파서로 지표 산출(LLM 실패/재시도/시간)
2. **가드레일 보강**  
   - 중복 ID/금지어/레어리티 스큐 검출을 로그 카운트로 남기기  
   - 품질 경고율 <5% 목표
3. **배포 패키지**  
   - Unity `.unitypackage` + C++ 빌드/프롬프트/설정 번들 생성

---

## 🎯 중기 (1~2주)
1. **파일럿 실행**  
   - 50~100개/타입, 프리셋 1~2개 → 임포트/QA/메트릭 수집 → 피드백 반영  
2. **배치 스트레스**  
   - `docs/BATCH_STRESS_PLAN.md` 재검증, 임포트/QA까지 포함  
3. **오류 처리/재시도 정책**  
   - LLM 호출/파싱/임포트 실패율 로그 기반으로 타임아웃·재시도 조정

---

## 🌱 옵션 (필요 시)
- Unreal 연동 메모 업데이트
- Prompt 템플릿/설정 파일 구조 유연화
- GUI 튜토리얼(스크린샷) 추가


