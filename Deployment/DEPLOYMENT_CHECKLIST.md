# 배포 체크리스트

## 필수 파일 확인

### 실행 파일
- [ ] RundeeItemFactory.exe 존재

### 설정 파일
- [ ] config/rundee_config.json 존재
- [ ] prompts/ 폴더 및 모든 프롬프트 파일 존재
  - [ ] prompts/food.txt
  - [ ] prompts/drink.txt
  - [ ] prompts/material.txt
  - [ ] prompts/weapon.txt
  - [ ] prompts/weapon_component.txt
  - [ ] prompts/ammo.txt

### 문서
- [ ] README.md 존재

## 의존성 확인

- [ ] Ollama 설치 및 실행 중
- [ ] Visual C++ Redistributable 설치 (필요한 경우)

## 테스트

- [ ] 실행 파일이 정상적으로 실행되는지 확인
- [ ] Ollama 연결 테스트
- [ ] 샘플 아이템 생성 테스트

## 배포 정보

- 빌드 구성: Release
- 플랫폼: x64
- 패키지 준비 일시: 2025-12-16 13:14:09
- 파일 수: 10
