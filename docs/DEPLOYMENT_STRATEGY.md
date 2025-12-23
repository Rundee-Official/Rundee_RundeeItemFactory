# RundeeItemFactory 배포 전략

## 개요

RundeeItemFactory는 두 가지 구성 요소로 나뉩니다:
1. **Unity 에디터 스크립트** (C#) - Unity Asset Store 패키지로 배포
2. **C++ 실행 파일** (.exe) - GitHub Releases로 별도 배포

## 배포 구조

### 1. Unity Asset Store 패키지

**포함 내용:**
- `Assets/RundeeItemFactory/Editor/` - 모든 Unity 에디터 스크립트
- `Assets/RundeeItemFactory/Runtime/` - 런타임 스크립트
- `Assets/RundeeItemFactory/ItemProfiles/` - 기본 프로필 (선택사항)
- `Assets/RundeeItemFactory/PlayerProfiles/` - 기본 플레이어 프로필 (선택사항)

**설치 후:**
- Unity 메뉴에 `Tools/Rundee/Item Factory/` 항목 추가
- Item Factory 창 사용 가능
- .exe 파일은 자동 다운로드 또는 수동 설치

### 2. C++ 실행 파일 배포

**GitHub Releases 배포:**
- `RundeeItemFactory.exe` - 메인 실행 파일
- `install_rundee_itemfactory.bat` - 설치 스크립트
- `config/rundee_config.json` - 기본 설정 파일 (선택사항)
- `README.md` - 사용 설명서

**의존성:**
- ✅ **필수**: Ollama (별도 설치 필요)
- ✅ **필수**: Visual C++ Redistributable (대부분 Windows에 이미 설치됨)
- ⚠️ **선택**: `config/rundee_config.json` (없어도 기본값 사용)
- ⚠️ **선택**: `profiles/` 폴더 (Unity에서 자동 복사됨)

**결론: .exe만 배포해도 작동 가능!**

## 설치 방법

### 방법 1: 자동 다운로드 (권장)

Unity 에디터에서:
1. Item Factory 창 열기
2. "Download Executable" 버튼 클릭
3. GitHub Releases에서 자동 다운로드
4. `{프로젝트 루트}/RundeeItemFactory.exe`에 설치됨

### 방법 2: 수동 설치 (배치 파일)

1. GitHub Releases에서 다운로드:
   - `RundeeItemFactory.exe`
   - `install_rundee_itemfactory.bat`

2. 배치 파일 실행:
   ```batch
   install_rundee_itemfactory.bat
   ```

3. 설치 위치:
   - `Assets/RundeeItemFactory/RundeeItemFactory.exe`

### 방법 3: 수동 설치 (직접 복사)

1. GitHub Releases에서 `RundeeItemFactory.exe` 다운로드
2. Unity 프로젝트의 `Assets/RundeeItemFactory/` 폴더에 `.exe` 파일 복사
3. (선택사항) `Assets/RundeeItemFactory/config/rundee_config.json` 생성
4. Unity 에디터에서 "Auto-Detect" 버튼으로 경로 자동 감지

## 권장 설치 위치

### 옵션 1: 프로젝트 루트 (권장)
```
{프로젝트 루트}/
  ├── UnityRundeeItemFactory/
  │   └── Assets/
  └── RundeeItemFactory/
      ├── RundeeItemFactory.exe
      └── config/
          └── rundee_config.json
```

**장점:**
- Unity 프로젝트와 분리
- Git에서 제외하기 쉬움
- 여러 Unity 프로젝트에서 공유 가능

### 옵션 2: Unity 프로젝트 내부
```
UnityRundeeItemFactory/
  └── RundeeItemFactory/
      ├── RundeeItemFactory.exe
      └── config/
          └── rundee_config.json
```

**장점:**
- 프로젝트와 함께 관리
- 프로젝트별로 다른 버전 사용 가능

## .exe 파일 구조

실행 파일은 다음 구조를 기대합니다:

```
Assets/RundeeItemFactory/
  ├── RundeeItemFactory.exe
  ├── config/
  │   └── rundee_config.json (선택사항)
  └── profiles/ (런타임에 생성됨, Unity에서 프로필 복사)
      └── *.json
```

**참고:** `profiles/` 폴더는 Unity 에디터가 자동으로 생성하고 프로필을 복사합니다.

## Unity 에디터 통합

Unity 에디터는 다음 순서로 .exe를 찾습니다:

1. **저장된 경로** (EditorPrefs)
2. **자동 감지**:
   - `Assets/RundeeItemFactory/RundeeItemFactory.exe` (권장, 배포된 환경)
   - `{프로젝트 루트}/RundeeItemFactory/RundeeItemFactory/x64/Release/RundeeItemFactory.exe` (개발 환경)
   - `{프로젝트 루트}/RundeeItemFactory/RundeeItemFactory/x64/Debug/RundeeItemFactory.exe` (개발 환경)
3. **다운로드** (GitHub Releases)

## 배포 체크리스트

### Unity Asset Store 패키지
- [ ] Unity 패키지 파일 생성 (.unitypackage)
- [ ] 패키지 매니페스트 작성
- [ ] 문서 및 스크린샷 준비
- [ ] Asset Store 제출

### GitHub Releases
- [ ] Release 태그 생성
- [ ] `RundeeItemFactory.exe` 업로드
- [ ] `install_rundee_itemfactory.bat` 업로드
- [ ] `config/rundee_config.json` 업로드 (선택사항)
- [ ] Release Notes 작성

### 문서
- [ ] README.md 업데이트
- [ ] 설치 가이드 작성
- [ ] 사용법 문서 작성

## 사용자 가이드

### 최소 설치 요구사항
1. Unity 2020.3 이상
2. Ollama 설치 및 실행
3. .exe 파일 (자동 다운로드 또는 수동 설치)

### 설치 후 확인
1. Unity 에디터에서 `Tools/Rundee/Item Factory/Item Factory` 메뉴 열기
2. "Auto-Detect" 버튼으로 .exe 경로 확인
3. 프로필 생성 및 아이템 생성 테스트


## 개요

RundeeItemFactory는 두 가지 구성 요소로 나뉩니다:
1. **Unity 에디터 스크립트** (C#) - Unity Asset Store 패키지로 배포
2. **C++ 실행 파일** (.exe) - GitHub Releases로 별도 배포

## 배포 구조

### 1. Unity Asset Store 패키지

**포함 내용:**
- `Assets/RundeeItemFactory/Editor/` - 모든 Unity 에디터 스크립트
- `Assets/RundeeItemFactory/Runtime/` - 런타임 스크립트
- `Assets/RundeeItemFactory/ItemProfiles/` - 기본 프로필 (선택사항)
- `Assets/RundeeItemFactory/PlayerProfiles/` - 기본 플레이어 프로필 (선택사항)

**설치 후:**
- Unity 메뉴에 `Tools/Rundee/Item Factory/` 항목 추가
- Item Factory 창 사용 가능
- .exe 파일은 자동 다운로드 또는 수동 설치

### 2. C++ 실행 파일 배포

**GitHub Releases 배포:**
- `RundeeItemFactory.exe` - 메인 실행 파일
- `install_rundee_itemfactory.bat` - 설치 스크립트
- `config/rundee_config.json` - 기본 설정 파일 (선택사항)
- `README.md` - 사용 설명서

**의존성:**
- ✅ **필수**: Ollama (별도 설치 필요)
- ✅ **필수**: Visual C++ Redistributable (대부분 Windows에 이미 설치됨)
- ⚠️ **선택**: `config/rundee_config.json` (없어도 기본값 사용)
- ⚠️ **선택**: `profiles/` 폴더 (Unity에서 자동 복사됨)

**결론: .exe만 배포해도 작동 가능!**

## 설치 방법

### 방법 1: 자동 다운로드 (권장)

Unity 에디터에서:
1. Item Factory 창 열기
2. "Download Executable" 버튼 클릭
3. GitHub Releases에서 자동 다운로드
4. `{프로젝트 루트}/RundeeItemFactory.exe`에 설치됨

### 방법 2: 수동 설치 (배치 파일)

1. GitHub Releases에서 다운로드:
   - `RundeeItemFactory.exe`
   - `install_rundee_itemfactory.bat`

2. 배치 파일 실행:
   ```batch
   install_rundee_itemfactory.bat
   ```

3. 설치 위치:
   - `Assets/RundeeItemFactory/RundeeItemFactory.exe`

### 방법 3: 수동 설치 (직접 복사)

1. GitHub Releases에서 `RundeeItemFactory.exe` 다운로드
2. Unity 프로젝트의 `Assets/RundeeItemFactory/` 폴더에 `.exe` 파일 복사
3. (선택사항) `Assets/RundeeItemFactory/config/rundee_config.json` 생성
4. Unity 에디터에서 "Auto-Detect" 버튼으로 경로 자동 감지

## 권장 설치 위치

### 옵션 1: 프로젝트 루트 (권장)
```
{프로젝트 루트}/
  ├── UnityRundeeItemFactory/
  │   └── Assets/
  └── RundeeItemFactory/
      ├── RundeeItemFactory.exe
      └── config/
          └── rundee_config.json
```

**장점:**
- Unity 프로젝트와 분리
- Git에서 제외하기 쉬움
- 여러 Unity 프로젝트에서 공유 가능

### 옵션 2: Unity 프로젝트 내부
```
UnityRundeeItemFactory/
  └── RundeeItemFactory/
      ├── RundeeItemFactory.exe
      └── config/
          └── rundee_config.json
```

**장점:**
- 프로젝트와 함께 관리
- 프로젝트별로 다른 버전 사용 가능

## .exe 파일 구조

실행 파일은 다음 구조를 기대합니다:

```
Assets/RundeeItemFactory/
  ├── RundeeItemFactory.exe
  ├── config/
  │   └── rundee_config.json (선택사항)
  └── profiles/ (런타임에 생성됨, Unity에서 프로필 복사)
      └── *.json
```

**참고:** `profiles/` 폴더는 Unity 에디터가 자동으로 생성하고 프로필을 복사합니다.

## Unity 에디터 통합

Unity 에디터는 다음 순서로 .exe를 찾습니다:

1. **저장된 경로** (EditorPrefs)
2. **자동 감지**:
   - `Assets/RundeeItemFactory/RundeeItemFactory.exe` (권장, 배포된 환경)
   - `{프로젝트 루트}/RundeeItemFactory/RundeeItemFactory/x64/Release/RundeeItemFactory.exe` (개발 환경)
   - `{프로젝트 루트}/RundeeItemFactory/RundeeItemFactory/x64/Debug/RundeeItemFactory.exe` (개발 환경)
3. **다운로드** (GitHub Releases)

## 배포 체크리스트

### Unity Asset Store 패키지
- [ ] Unity 패키지 파일 생성 (.unitypackage)
- [ ] 패키지 매니페스트 작성
- [ ] 문서 및 스크린샷 준비
- [ ] Asset Store 제출

### GitHub Releases
- [ ] Release 태그 생성
- [ ] `RundeeItemFactory.exe` 업로드
- [ ] `install_rundee_itemfactory.bat` 업로드
- [ ] `config/rundee_config.json` 업로드 (선택사항)
- [ ] Release Notes 작성

### 문서
- [ ] README.md 업데이트
- [ ] 설치 가이드 작성
- [ ] 사용법 문서 작성

## 사용자 가이드

### 최소 설치 요구사항
1. Unity 2020.3 이상
2. Ollama 설치 및 실행
3. .exe 파일 (자동 다운로드 또는 수동 설치)

### 설치 후 확인
1. Unity 에디터에서 `Tools/Rundee/Item Factory/Item Factory` 메뉴 열기
2. "Auto-Detect" 버튼으로 .exe 경로 확인
3. 프로필 생성 및 아이템 생성 테스트
