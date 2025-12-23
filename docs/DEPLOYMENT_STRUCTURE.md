# 배포 구조 가이드

이 문서는 RundeeItemFactory를 출시할 때 어떤 파일들이 어떤 구조로 배포되는지 설명합니다.

## 배포 패키지 구조

### 1. C++ 실행 파일 패키지 (Standalone)

사용자가 C++ 실행 파일만 사용하는 경우:

```
RundeeItemFactory/
├── RundeeItemFactory.exe          # 메인 실행 파일
├── config/
│   └── rundee_config.json         # Ollama 설정 파일 (선택사항)
├── profiles/                       # 사용자 프로필 저장 폴더 (자동 생성)
│   └── (사용자가 만든 프로필들)
├── Registry/                       # ID 레지스트리 (자동 생성)
├── ItemJson/                       # 생성된 JSON 파일 저장 폴더 (자동 생성)
├── README.md                       # 사용 설명서
└── DEPLOYMENT_CHECKLIST.md         # 배포 체크리스트
```

**필수 파일:**
- `RundeeItemFactory.exe`

**선택 파일:**
- `config/rundee_config.json` (기본값 사용 가능)
- `README.md`

**자동 생성 폴더:**
- `profiles/` - 프로필 저장 시 자동 생성
- `Registry/` - ID 레지스트리 저장 시 자동 생성
- `ItemJson/` - 생성된 JSON 파일 저장 시 자동 생성

### 2. Unity 패키지 (Unity Integration)

Unity 프로젝트에 통합하는 경우:

#### 옵션 A: Unity Package로 배포

```
RundeeItemFactory.unitypackage
└── Assets/
    └── RundeeItemFactory/
        ├── Editor/
        │   ├── ItemFactoryMainWindow.cs
        │   ├── ItemProfileManagerWindow.cs
        │   ├── JsonImportMainWindow.cs
        │   ├── SetupItemFactoryMenu.cs
        │   ├── DynamicItemImporter.cs
        │   ├── ItemImporter.cs
        │   └── (기타 Editor 스크립트들)
        └── Runtime/
            ├── ItemProfile.cs
            ├── ItemDatabase.cs
            ├── FoodItemDataSO.cs
            ├── DrinkItemDataSO.cs
            ├── MaterialItemDataSO.cs
            ├── WeaponItemDataSO.cs
            ├── WeaponComponentItemDataSO.cs
            ├── AmmoItemDataSO.cs
            └── (기타 Runtime 스크립트들)
```

#### 옵션 B: 전체 프로젝트로 배포

```
RundeeItemFactory-Complete/
├── RundeeItemFactory/              # C++ 프로젝트
│   └── (C++ 소스 코드)
├── UnityRundeeItemFactory/          # Unity 프로젝트
│   └── Assets/
│       └── RundeeItemFactory/
│           └── (Unity 스크립트들)
├── profiles/                        # 프로필 저장 폴더
│   └── (사용자 프로필들)
├── scripts/                         # 빌드/테스트 스크립트
└── docs/                            # 문서
```

### 3. Unity Asset Store 배포

**중요**: Unity Asset Store는 실행 파일(.exe)을 직접 포함할 수 없습니다.

#### 옵션 A: Unity 패키지 + 별도 다운로드 (권장)

```
Asset Store 배포:
└── RundeeItemFactory.unitypackage   # Unity 패키지만 Asset Store에 업로드

별도 제공 (GitHub/웹사이트):
├── RundeeItemFactory.exe            # 실행 파일 다운로드
├── config/
└── README.md
```

**배포 방법:**
1. Unity 패키지(`.unitypackage`)를 Asset Store에 업로드
2. Asset Store 설명에 실행 파일 다운로드 링크 추가
3. 사용자는 Unity 패키지 설치 후 실행 파일을 별도로 다운로드

**장점:**
- Asset Store 정책 준수
- Unity 패키지는 자동 업데이트 가능
- 실행 파일은 독립적으로 버전 관리 가능

**단점:**
- 사용자가 두 단계로 설치해야 함
- 실행 파일 경로를 수동으로 설정해야 할 수 있음

#### 옵션 B: Unity 패키지 내 자동 다운로드 (구현 완료) ✅

Unity 패키지에 실행 파일 자동 다운로드 기능이 포함되어 있습니다:

```
RundeeItemFactory.unitypackage
└── Assets/RundeeItemFactory/
    ├── Editor/
    │   ├── ItemFactoryMainWindow.cs    # 다운로드 UI 통합됨
    │   ├── ExecutableDownloader.cs     # 실행 파일 자동 다운로드
    │   └── (기존 스크립트들)
    └── Runtime/
        └── (기존 스크립트들)
```

**구현된 기능:**
- ✅ GitHub Releases에서 실행 파일 자동 다운로드
- ✅ 다운로드 진행률 표시
- ✅ 다운로드 완료 후 자동 경로 설정
- ✅ 버전 확인 기능
- ✅ 업데이트 확인 기능
- ✅ 다운로드 실패 시 에러 처리 및 백업 복원

**사용 방법:**
1. Unity Asset Store에서 패키지 설치
2. `Tools > Rundee > Item Factory > Generation > Item Factory` 열기
3. "Download Executable" 버튼 클릭
4. 다운로드 완료 후 자동으로 설정됨

**설정 필요:**
- `ExecutableDownloader.cs`의 `GITHUB_RELEASES_URL`을 실제 GitHub Releases URL로 업데이트
- 또는 CDN URL로 변경 가능

**장점:**
- ✅ 사용자가 한 번만 설치하면 됨
- ✅ 자동화된 설정
- ✅ 업데이트 확인 가능

**주의사항:**
- 네트워크 연결 필요
- GitHub Releases에 실행 파일 업로드 필요
- Asset Store 정책 준수 (외부 파일 다운로드는 허용됨)

### 4. 하이브리드 배포 (직접 배포)

C++ 실행 파일과 Unity 패키지를 함께 제공 (Asset Store 외):

```
RundeeItemFactory-v1.0/
├── Standalone/                      # 독립 실행 패키지
│   ├── RundeeItemFactory.exe
│   ├── config/
│   └── README.md
├── Unity/                           # Unity 통합 패키지
│   └── RundeeItemFactory.unitypackage
├── profiles/                        # 예제 프로필 (선택사항)
│   └── (예제 프로필 파일들)
└── docs/                            # 문서
    ├── INSTALLATION_GUIDE.md
    ├── UNITY_IMPORT_GUIDE.md
    └── README.md
```

## 파일 설명

### 필수 파일

1. **RundeeItemFactory.exe**
   - 메인 실행 파일
   - Windows x64 플랫폼용
   - Visual C++ Redistributable 필요 (일반적으로 이미 설치됨)

### 선택 파일

1. **config/rundee_config.json**
   - Ollama 연결 설정
   - 없으면 기본값 사용 (localhost:11434)

2. **profiles/** 폴더
   - 사용자가 만든 커스텀 프로필
   - 프로젝트 루트에 자동 생성됨
   - 배포 시 예제 프로필 포함 가능

### 자동 생성 폴더

1. **profiles/** (프로젝트 루트)
   - 프로필 저장 시 자동 생성
   - 경로: `{프로젝트 루트}/profiles/`

2. **Registry/** (실행 파일 옆)
   - ID 레지스트리 저장
   - 중복 ID 방지용

3. **ItemJson/** (프로젝트 루트)
   - 생성된 JSON 파일 저장
   - 기본 출력 경로

## 배포 시나리오별 구조

### 시나리오 1: CLI만 사용하는 사용자

```
사용자 폴더/
└── RundeeItemFactory/
    ├── RundeeItemFactory.exe
    ├── config/ (선택사항)
    └── profiles/ (자동 생성)
```

### 시나리오 2: Unity 통합 사용자

```
Unity 프로젝트/
├── Assets/
│   └── RundeeItemFactory/          # Unity 패키지 설치됨
└── (프로젝트 루트)/
    ├── RundeeItemFactory.exe       # Unity에서 호출
    └── profiles/                    # 프로필 저장 (자동 생성)
```

### 시나리오 3: 개발자 (소스 포함)

```
RundeeItemFactory/
├── RundeeItemFactory/               # C++ 소스 (Visual Studio 프로젝트)
│   └── RundeeItemFactory.sln
├── docs/                            # 문서
├── LICENCES/                        # 라이선스 파일
└── README.md

참고: 다음 폴더는 테스트/개발용이며 git에 추적되지 않습니다:
- scripts/                           # 빌드/테스트 스크립트
- UnityRundeeItemFactory/            # Unity 테스트 프로젝트
```

## 배포 체크리스트

### C++ 실행 파일 패키지

- [ ] `RundeeItemFactory.exe` (Release 빌드)
- [ ] `config/rundee_config.json` (선택사항)
- [ ] `README.md`
- [ ] Visual C++ Redistributable 설치 안내

### Unity 패키지

- [ ] `.unitypackage` 파일 생성
- [ ] 모든 Editor 스크립트 포함
- [ ] 모든 Runtime 스크립트 포함
- [ ] 메타 파일 포함 확인
- [ ] Unity 버전 호환성 확인 (2020.3+)

### 문서

- [ ] 설치 가이드
- [ ] 사용 설명서
- [ ] Unity 통합 가이드
- [ ] 문제 해결 가이드

## 배포 스크립트 사용

배포 패키지를 준비하려면:

```powershell
.\scripts\prepare_deployment_package.ps1 -Configuration Release -Platform x64
```

이 스크립트는 `Deployment/` 폴더에 배포 패키지를 생성합니다.

## 주의사항

1. **프로필 경로**: 프로필은 프로젝트 루트의 `profiles/` 폴더에 저장됩니다.
2. **실행 파일 경로**: Unity에서 실행 파일 경로를 자동 감지하지만, 수동 설정도 가능합니다.
3. **의존성**: Ollama가 설치되어 있어야 LLM 기능을 사용할 수 있습니다.
4. **플랫폼**: 현재 Windows x64만 지원됩니다.
5. **프롬프트 파일**: 더 이상 사용하지 않습니다. 프로필 기반 시스템에서 프로필의 필드와 customContext로 동적으로 프롬프트를 생성합니다.
6. **프로필 기반 시스템**: LLM 프롬프트는 프로필 데이터에서 동적으로 생성됩니다. 모든 필드 정의, 검증 규칙, 관계 제약, 플레이어 설정이 프롬프트에 포함됩니다.
