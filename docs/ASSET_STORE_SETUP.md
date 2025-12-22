# Unity Asset Store 배포 설정 가이드

이 가이드는 Unity Asset Store에 RundeeItemFactory를 배포하기 위한 설정 방법을 설명합니다.

## 자동 다운로드 기능

Unity 패키지에는 실행 파일 자동 다운로드 기능이 포함되어 있습니다. 사용자는 Asset Store에서 패키지를 설치한 후, Unity Editor 내에서 실행 파일을 자동으로 다운로드할 수 있습니다.

## 설정 방법

### 1. GitHub Releases에 실행 파일 업로드

1. GitHub 저장소의 Releases 페이지로 이동
2. 새 Release 생성 (예: v1.0.0)
3. `RundeeItemFactory.exe` 파일을 Assets에 추가
4. Release 게시

### 2. ExecutableDownloader.cs 설정

`Assets/RundeeItemFactory/Editor/ExecutableDownloader.cs` 파일을 열고 다음을 수정:

```csharp
// GitHub Releases URL 업데이트
private const string GITHUB_RELEASES_URL = "https://github.com/YOUR_USERNAME/RundeeItemFactory/releases/latest/download/RundeeItemFactory.exe";

// GitHub Repository 정보 업데이트
private const string GITHUB_REPO = "YOUR_USERNAME/RundeeItemFactory";
```

**URL 형식:**
- Latest 버전: `https://github.com/USERNAME/REPO/releases/latest/download/RundeeItemFactory.exe`
- 특정 버전: `https://github.com/USERNAME/REPO/releases/download/v1.0.0/RundeeItemFactory.exe`

### 3. CDN 사용 (선택사항)

GitHub Releases 대신 CDN을 사용하려면:

```csharp
// GITHUB_RELEASES_URL 대신 CDN_URL 사용
private const string CDN_URL = "https://your-cdn.com/RundeeItemFactory.exe";

// DownloadUrl 프로퍼티 수정
public static string DownloadUrl => CDN_URL;
```

## Unity 패키지 생성

1. Unity Editor에서 프로젝트 열기
2. `Assets > Export Package...` 선택
3. 다음 폴더/파일 포함:
   - `Assets/RundeeItemFactory/Editor/` (모든 Editor 스크립트)
   - `Assets/RundeeItemFactory/Runtime/` (모든 Runtime 스크립트)
4. `.unitypackage` 파일로 내보내기

## Asset Store 업로드

1. Unity Asset Store Publisher Portal에 로그인
2. 새 패키지 생성 또는 기존 패키지 업데이트
3. `.unitypackage` 파일 업로드
4. 패키지 설명에 다음 정보 추가:
   - 실행 파일은 Unity Editor 내에서 자동으로 다운로드됩니다
   - "Download Executable" 버튼을 클릭하여 다운로드
   - GitHub Releases 링크 (선택사항)

## 사용자 경험

### 사용자 설치 과정

1. **Asset Store에서 패키지 설치**
   - Unity Package Manager를 통해 설치
   - 또는 `.unitypackage` 파일 직접 임포트

2. **실행 파일 다운로드**
   - `Tools > Rundee > Item Factory > Generation > Item Factory` 메뉴 열기
   - "Download Executable" 버튼 클릭
   - 다운로드 진행률 표시
   - 완료 후 자동으로 경로 설정

3. **사용 시작**
   - 프로필 선택 및 아이템 생성 시작

## 보안 고려사항

- ✅ GitHub Releases는 공개적으로 검증 가능한 소스
- ✅ HTTPS를 통한 안전한 다운로드
- ✅ 다운로드 실패 시 기존 파일 백업 복원
- ⚠️ Asset Store 정책 확인 필요 (외부 파일 다운로드 허용 여부)

## 문제 해결

### 다운로드 실패 시

1. 네트워크 연결 확인
2. GitHub Releases URL이 올바른지 확인
3. 파일이 실제로 업로드되었는지 확인
4. 수동 다운로드 옵션 제공 (Browse 버튼 사용)

### 버전 확인 실패 시

- GitHub API rate limit 확인
- Repository가 public인지 확인
- GITHUB_REPO 설정이 올바른지 확인

## 업데이트 프로세스

새 버전의 실행 파일을 배포할 때:

1. GitHub Releases에 새 버전 업로드
2. Unity 패키지 업데이트 (필요한 경우)
3. Asset Store에 새 버전 업로드
4. 사용자는 "Check for Updates" 버튼으로 새 버전 확인 가능

