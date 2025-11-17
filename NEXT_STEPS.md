# 다음 단계 제안

## 📊 현재 상태

### ✅ 완료된 것들
- **Phase 0-1 핵심 기능**: 모두 완료
  - 로컬 LLM 연동 (Ollama)
  - Food/Drink/Material JSON 생성
  - Unity Food/Material 임포트
- **코드 구조**: 깔끔하게 정리됨
  - 폴더별 분리 (Data, Validators, Parsers, Writers, etc.)
  - OOP 구조 (ItemDataBase 상속)
  - Utils 모음
  - 메인 함수 간소화 (358줄 → 38줄)
- **Drink 타입**: 별도로 분리 완료

---

## 🎯 다음 단계 제안 (우선순위별)

### 🔥 **즉시 할 수 있는 것들** (1-2일)

#### 1. **Unity Drink 통합** ⭐ (가장 빠름)
- 현재: Food, Material만 Unity 임포트 가능
- 필요: `DrinkItemImporter.cs`, `DrinkItemDataSO.cs`, `DrinkItemDatabase.cs`
- 예상 시간: 30분~1시간
- **추천 이유**: Food/Material과 동일한 패턴이라 빠르게 완료 가능

#### 2. **밸런스 리포트 기능** ⭐⭐
- 생성된 아이템의 통계 확인
- 평균/최소/최대 수치
- 레어리티별 통계
- 밸런스 이상 감지
- 예상 시간: 2-3시간
- **추천 이유**: 실제 사용 시 유용, 구현 난이도 낮음

#### 3. **재시도 로직 추가**
- LLM 응답 실패 시 자동 재시도
- 타임아웃 설정
- 예상 시간: 1-2시간
- **추천 이유**: 안정성 향상

---

### 🚀 **Phase 2 핵심 기능** (1-2주)

#### 4. **Unity EditorWindow 통합** ⭐⭐⭐ (가장 중요)
- GUI로 모델 선택, 프리셋 선택, 파라미터 입력
- "Generate" 버튼으로 한 번에 실행
- 생성 로그 표시
- 예상 시간: 1-2일
- **추천 이유**: Phase 2의 핵심, 사용성 대폭 향상

**구현 예시:**
```csharp
// Unity EditorWindow
public class ItemFactoryWindow : EditorWindow
{
    private string modelName = "llama3";
    private PresetType preset = PresetType.City;
    private ItemType itemType = ItemType.Food;
    private int count = 10;
    
    [MenuItem("Tools/Rundee/Item Factory")]
    public static void ShowWindow() { ... }
    
    void OnGUI() { ... }
}
```

#### 5. **배치 생성 안정화**
- 50-200개 아이템 생성 테스트
- 부분 실패 시 성공한 아이템만 저장
- 진행률 표시
- 예상 시간: 1일

#### 6. **아이템 관리 기능**
- 이미 생성된 아이템 목록 표시
- 필터 (카테고리/레어리티/테마)
- 선택한 아이템만 재생성/삭제
- 예상 시간: 2-3일

---

### 🔧 **기술적 개선** (선택사항)

#### 7. **LLM HTTP API 직접 호출**
- 현재: `_popen("ollama run ...")`
- 개선: HTTP POST to `http://localhost:11434/api/generate`
- 장점: 더 빠르고 안정적, 스트리밍 지원
- 예상 시간: 반나절
- **주의**: HTTP 라이브러리 필요 (libcurl 또는 Windows HTTP API)

#### 8. **설정 파일 시스템**
- 프롬프트 템플릿을 외부 파일로 분리
- JSON/YAML 설정 파일
- 예상 시간: 반나절

---

## 💡 추천 순서

### **옵션 A: 빠른 완성** (1-2일)
1. Unity Drink 통합 (30분)
2. 밸런스 리포트 (2-3시간)
3. 재시도 로직 (1-2시간)

### **옵션 B: Phase 2 진입** (1-2주)
1. Unity EditorWindow 통합 (1-2일) ⭐
2. 배치 생성 안정화 (1일)
3. 아이템 관리 기능 (2-3일)

### **옵션 C: 기술적 개선** (선택)
1. LLM HTTP API 직접 호출 (반나절)
2. 설정 파일 시스템 (반나절)

---

## 🎯 내 추천

**지금 바로 할 것:**
1. **Unity Drink 통합** - 빠르고 완성도 높임
2. **밸런스 리포트** - 실제 사용 시 유용

**다음 주 할 것:**
3. **Unity EditorWindow 통합** - Phase 2의 핵심 기능

어떤 것부터 시작할까요? 🤔

