# RundeeItemFactory API Documentation

이 디렉토리는 RundeeItemFactory 프로젝트의 API 문서를 포함합니다.

## 문서 생성 방법

### C++ 문서 (Doxygen)

1. **Doxygen 설치**
   - https://www.doxygen.nl/download.html 에서 다운로드
   - Windows: 설치 후 기본 경로는 `C:\Program Files\doxygen\bin\doxygen.exe`

2. **문서 생성**
   ```powershell
   # 프로젝트 루트에서 실행
   .\scripts\generate_docs.ps1
   
   # 또는 Doxygen 직접 실행
   doxygen Doxyfile
   ```

3. **문서 확인**
   - 생성된 HTML 문서: `docs/cpp/html/index.html`
   - 브라우저에서 열어서 확인

### C# 문서 (XML Documentation Comments)

1. **Unity에서 XML 생성 활성화**
   - Unity Editor → Edit → Project Settings → Player
   - Other Settings → Generate XML Documentation Files 체크

2. **문서 위치**
   - XML 파일: `UnityRundeeItemFactory/Library/ScriptAssemblies/*.xml`
   - Visual Studio나 다른 IDE에서 IntelliSense로 확인 가능

## 문서 구조

### C++ API 문서

- **네임스페이스**
  - `ItemGenerator`: 메인 아이템 생성 로직
  - `CommandLineParser`: 명령줄 인자 파싱
  - `ItemJsonParser`: JSON 파싱
  - `ItemJsonWriter`: JSON 쓰기
  - `OllamaClient`: LLM 통신

- **주요 클래스**
  - `ItemGenerator`: 아이템 생성 메인 클래스
  - `ItemJsonParser`: JSON 파서
  - `ItemJsonWriter`: JSON 라이터
  - `OllamaClient`: Ollama LLM 클라이언트
  - `FoodItemValidator`, `DrinkItemValidator`, etc.: 아이템 검증기

### C# Unity API 문서

- **Editor Windows**
  - `ItemFactoryWindow`: 아이템 생성 윈도우
  - `ItemManagerWindow`: 아이템 관리 윈도우
  - `JsonImportWindow`: JSON 임포트 윈도우

- **Runtime ScriptableObjects**
  - `FoodItemDataSO`, `DrinkItemDataSO`, etc.: 아이템 데이터 ScriptableObject

## 문서 스타일 가이드

### C++ (Doxygen)

```cpp
/**
 * @brief 함수의 간단한 설명
 * 
 * 자세한 설명은 여기에 작성합니다.
 * 
 * @param paramName 매개변수 설명
 * @param[out] outputParam 출력 매개변수 설명
 * @return 반환값 설명
 * @note 추가 참고사항
 * @see 관련 함수나 클래스
 */
```

### C# (XML Documentation)

```csharp
/// <summary>
/// 함수의 간단한 설명
/// </summary>
/// <param name="paramName">매개변수 설명</param>
/// <returns>반환값 설명</returns>
/// <remarks>
/// 자세한 설명은 여기에 작성합니다.
/// </remarks>
```

## 문서 업데이트

코드 변경 후 문서를 업데이트하려면:

1. 코드에 주석 추가/수정
2. 문서 재생성: `.\scripts\generate_docs.ps1`
3. 변경사항 확인: `docs/cpp/html/index.html`

## 참고 자료

- [Doxygen 공식 문서](https://www.doxygen.nl/manual/index.html)
- [C# XML Documentation](https://docs.microsoft.com/en-us/dotnet/csharp/language-reference/xmldoc/)





