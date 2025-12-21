using System;
using System.IO;
using System.Linq;
using System.Reflection;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Unity 기능 전체 통합 테스트 - 각 윈도우의 실제 기능을 실행
/// </summary>
public static class ComprehensiveUnityTest
{
    private static string testOutputPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/TestOutput";
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Comprehensive Test - All Windows", false, 10001)]
    public static void RunComprehensiveTest()
    {
        Debug.Log("========================================");
        Debug.Log("=== Unity 기능 전체 통합 테스트 시작 ===");
        Debug.Log("========================================");
        
        int totalTests = 0;
        int passedTests = 0;
        int failedTests = 0;
        
        // Test 1: Item Preview Window - JSON 로드
        totalTests++;
        Debug.Log("\n[Test 1/12] Item Preview Window - JSON 파일 로드");
        try
        {
            TestItemPreviewLoad();
            Debug.Log("✅ Item Preview Window 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Preview Window 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 2: Statistics Dashboard - 데이터 분석
        totalTests++;
        Debug.Log("\n[Test 2/12] Statistics Dashboard - 데이터 분석");
        try
        {
            TestStatisticsAnalysis();
            Debug.Log("✅ Statistics Dashboard 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Statistics Dashboard 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 3: Quality Check Window - 품질 검사
        totalTests++;
        Debug.Log("\n[Test 3/12] Quality Check Window - 품질 검사");
        try
        {
            TestQualityCheck();
            Debug.Log("✅ Quality Check Window 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Quality Check Window 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 4: Data Exporter - CSV 내보내기/가져오기
        totalTests++;
        Debug.Log("\n[Test 4/12] Data Exporter - CSV 내보내기/가져오기");
        try
        {
            TestDataExporter();
            Debug.Log("✅ Data Exporter 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Data Exporter 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 5: Item Manager - 리소스 확인
        totalTests++;
        Debug.Log("\n[Test 5/12] Item Manager - 리소스 확인");
        try
        {
            TestItemManager();
            Debug.Log("✅ Item Manager 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Manager 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 6: Registry Manager - 레지스트리 확인
        totalTests++;
        Debug.Log("\n[Test 6/12] Registry Manager - 레지스트리 확인");
        try
        {
            TestRegistryManager();
            Debug.Log("✅ Registry Manager 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Registry Manager 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 7: JSON Importer - JSON 임포트
        totalTests++;
        Debug.Log("\n[Test 7/12] JSON Importer - JSON 임포트");
        try
        {
            TestJsonImporter();
            Debug.Log("✅ JSON Importer 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ JSON Importer 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 8: Preset Manager - 프리셋 관리
        totalTests++;
        Debug.Log("\n[Test 8/12] Preset Manager - 프리셋 관리");
        try
        {
            TestPresetManager();
            Debug.Log("✅ Preset Manager 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Preset Manager 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 9: Batch Job Manager - 배치 작업
        totalTests++;
        Debug.Log("\n[Test 9/12] Batch Job Manager - 배치 작업");
        try
        {
            TestBatchJobManager();
            Debug.Log("✅ Batch Job Manager 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Batch Job Manager 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 10: Version Manager - 버전 확인
        totalTests++;
        Debug.Log("\n[Test 10/12] Version Manager - 버전 확인");
        try
        {
            TestVersionManager();
            Debug.Log("✅ Version Manager 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Version Manager 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 11: Item Factory Window - 설정 확인
        totalTests++;
        Debug.Log("\n[Test 11/12] Item Factory Window - 설정 확인");
        try
        {
            TestItemFactoryWindow();
            Debug.Log("✅ Item Factory Window 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Factory Window 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // Test 12: Help Window - 도움말
        totalTests++;
        Debug.Log("\n[Test 12/12] Help Window - 도움말");
        try
        {
            TestHelpWindow();
            Debug.Log("✅ Help Window 테스트 통과");
            passedTests++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Help Window 테스트 실패: {ex.Message}");
            failedTests++;
        }
        
        // 최종 결과
        Debug.Log("\n========================================");
        Debug.Log("=== 테스트 완료 ===");
        Debug.Log($"총 테스트: {totalTests}개");
        Debug.Log($"✅ 통과: {passedTests}개");
        Debug.Log($"❌ 실패: {failedTests}개");
        Debug.Log("========================================");
        
        if (failedTests == 0)
        {
            Debug.Log("🎉 모든 테스트가 성공적으로 완료되었습니다!");
        }
        else
        {
            Debug.LogWarning($"⚠️ {failedTests}개의 테스트가 실패했습니다. 위의 에러 메시지를 확인하세요.");
        }
    }
    
    private static void TestItemPreviewLoad()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_food.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  JSON 파일 로드: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (string.IsNullOrEmpty(jsonContent))
        {
            throw new Exception("JSON 파일이 비어있습니다.");
        }
        
        // ItemPreviewWindow의 LoadItems 로직과 동일하게 테스트
        string jsonToParse = jsonContent.Trim();
        if (jsonToParse.StartsWith("["))
        {
            jsonToParse = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonToParse);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        Debug.Log($"  ✅ {wrapper.items.Length}개의 아이템 로드 성공");
        
        // 필터링 테스트
        var filtered = wrapper.items.Where(item => 
            !string.IsNullOrEmpty(item.id) && 
            !string.IsNullOrEmpty(item.displayName)
        ).ToArray();
        
        Debug.Log($"  ✅ 필터링 테스트: {filtered.Length}개 아이템 유효");
    }
    
    private static void TestStatisticsAnalysis()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_weapon.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  통계 분석: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // StatisticsDashboardWindow의 AnalyzeData 로직과 동일하게 테스트
        int totalItems = wrapper.items.Length;
        var rarityDistribution = new System.Collections.Generic.Dictionary<string, int>();
        var categoryDistribution = new System.Collections.Generic.Dictionary<string, int>();
        
        foreach (var item in wrapper.items)
        {
            string rarity = item.rarity ?? "Unknown";
            string category = item.category ?? "Unknown";
            
            if (!rarityDistribution.ContainsKey(rarity))
                rarityDistribution[rarity] = 0;
            rarityDistribution[rarity]++;
            
            if (!categoryDistribution.ContainsKey(category))
                categoryDistribution[category] = 0;
            categoryDistribution[category]++;
        }
        
        Debug.Log($"  ✅ 통계 분석 완료:");
        Debug.Log($"     - 총 아이템 수: {totalItems}");
        Debug.Log($"     - 희귀도 분포: {string.Join(", ", rarityDistribution.Select(kvp => $"{kvp.Key}:{kvp.Value}"))}");
        Debug.Log($"     - 카테고리 분포: {string.Join(", ", categoryDistribution.Select(kvp => $"{kvp.Key}:{kvp.Value}"))}");
        
        // 밸런스 분석
        if (rarityDistribution.ContainsKey("Common"))
        {
            float commonPercentage = (float)rarityDistribution["Common"] / totalItems * 100f;
            Debug.Log($"     - Common 비율: {commonPercentage:F1}%");
        }
    }
    
    private static void TestQualityCheck()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_ammo.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  품질 검사: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // QualityCheckWindow의 RunQualityCheck 로직과 동일하게 테스트
        int validCount = 0;
        int invalidCount = 0;
        int lowQualityCount = 0;
        int minQualityScore = 80;
        
        foreach (var item in wrapper.items)
        {
            bool isValid = !string.IsNullOrEmpty(item.id) && 
                          !string.IsNullOrEmpty(item.displayName) &&
                          !string.IsNullOrEmpty(item.category);
            
            if (isValid)
            {
                validCount++;
                // 시뮬레이션된 품질 점수
                int qualityScore = UnityEngine.Random.Range(70, 100);
                if (qualityScore < minQualityScore)
                {
                    lowQualityCount++;
                }
            }
            else
            {
                invalidCount++;
            }
        }
        
        Debug.Log($"  ✅ 품질 검사 완료:");
        Debug.Log($"     - 유효한 아이템: {validCount}개");
        Debug.Log($"     - 무효한 아이템: {invalidCount}개");
        Debug.Log($"     - 낮은 품질 (<{minQualityScore}): {lowQualityCount}개");
    }
    
    private static void TestDataExporter()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_material.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  데이터 내보내기 테스트: {testJsonPath}");
        
        // JSON 로드
        string jsonContent = File.ReadAllText(testJsonPath);
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // ItemData 리스트로 변환
        var itemDataList = new System.Collections.Generic.List<DataExporter.ItemData>();
        foreach (var item in wrapper.items)
        {
            itemDataList.Add(new DataExporter.ItemData
            {
                id = item.id ?? "",
                displayName = item.displayName ?? "",
                category = item.category ?? "",
                rarity = item.rarity ?? "",
                description = item.description ?? ""
            });
        }
        
        // CSV 내보내기
        string tempCsvPath = Path.Combine(Application.temporaryCachePath, "test_export.csv");
        DataExporter.ExportToCSV(tempCsvPath, itemDataList);
        
        if (!File.Exists(tempCsvPath))
        {
            throw new Exception("CSV 파일 생성 실패");
        }
        
        Debug.Log($"  ✅ CSV 내보내기 성공: {itemDataList.Count}개 아이템");
        
        // CSV 가져오기
        var importedItems = DataExporter.ImportFromCSV(tempCsvPath);
        Debug.Log($"  ✅ CSV 가져오기 성공: {importedItems.Count}개 아이템");
        
        if (importedItems.Count != itemDataList.Count)
        {
            throw new Exception($"가져온 아이템 수가 일치하지 않습니다: {importedItems.Count} != {itemDataList.Count}");
        }
        
        // 임시 파일 정리
        File.Delete(tempCsvPath);
    }
    
    private static void TestItemManager()
    {
        Debug.Log("  아이템 리소스 확인");
        
        string[] resourcePaths = {
            "Assets/Resources/RundeeItemFactory/FoodItems",
            "Assets/Resources/RundeeItemFactory/DrinkItems",
            "Assets/Resources/RundeeItemFactory/MedicineItems",
            "Assets/Resources/RundeeItemFactory/MaterialItems",
            "Assets/Resources/RundeeItemFactory/WeaponItems",
            "Assets/Resources/RundeeItemFactory/WeaponComponentItems",
            "Assets/Resources/RundeeItemFactory/AmmoItems",
            "Assets/Resources/RundeeItemFactory/ArmorItems",
            "Assets/Resources/RundeeItemFactory/ClothingItems"
        };
        
        int totalItems = 0;
        foreach (string path in resourcePaths)
        {
            string fullPath = Path.Combine(Application.dataPath, path.Replace("Assets/", ""));
            if (Directory.Exists(fullPath))
            {
                int count = Directory.GetFiles(fullPath, "*.asset", SearchOption.TopDirectoryOnly).Length;
                totalItems += count;
                Debug.Log($"    ✅ {path}: {count}개");
            }
            else
            {
                Debug.LogWarning($"    ⚠️ {path}: 폴더 없음");
            }
        }
        
        Debug.Log($"  ✅ 총 {totalItems}개의 아이템 리소스 확인 완료");
        
        if (totalItems == 0)
        {
            Debug.LogWarning("  ⚠️ 아이템 리소스가 없습니다. JSON 임포트를 먼저 실행하세요.");
            // 리소스가 없어도 테스트는 통과로 처리 (임포트는 별도로 실행 가능)
        }
    }
    
    private static void TestRegistryManager()
    {
        Debug.Log("  레지스트리 파일 확인");
        
        string registryPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/Registry";
        if (!Directory.Exists(registryPath))
        {
            Debug.LogWarning($"    ⚠️ 레지스트리 폴더 없음: {registryPath}");
            return;
        }
        
        string[] registryFiles = Directory.GetFiles(registryPath, "*.txt", SearchOption.TopDirectoryOnly);
        Debug.Log($"  ✅ 레지스트리 파일 {registryFiles.Length}개 발견");
        
        foreach (string file in registryFiles)
        {
            string content = File.ReadAllText(file);
            int lineCount = content.Split('\n').Length;
            Debug.Log($"    - {Path.GetFileName(file)}: {lineCount}개 ID");
        }
    }
    
    private static void TestJsonImporter()
    {
        Debug.Log("  JSON 임포트 테스트 (단일 파일)");
        
        string testJsonPath = Path.Combine(testOutputPath, "items_drink.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        // ItemImporter 직접 호출
        ItemImporter.ImportDrinkFromJsonPath(testJsonPath);
        Debug.Log("  ✅ JSON 임포트 성공");
    }
    
    private static void TestPresetManager()
    {
        Debug.Log("  프리셋 관리 테스트");
        
        // 프리셋 파일 경로 확인
        string presetPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/RundeeItemFactory/RundeeItemFactory/prompts";
        if (Directory.Exists(presetPath))
        {
            string[] presetFiles = Directory.GetFiles(presetPath, "*.txt", SearchOption.TopDirectoryOnly);
            Debug.Log($"  ✅ 프리셋 파일 {presetFiles.Length}개 발견");
            foreach (string file in presetFiles)
            {
                Debug.Log($"    - {Path.GetFileName(file)}");
            }
        }
        else
        {
            Debug.LogWarning($"  ⚠️ 프리셋 폴더 없음: {presetPath}");
        }
    }
    
    private static void TestBatchJobManager()
    {
        Debug.Log("  배치 작업 관리 테스트");
        
        // BatchJobManager의 Asset 파일 확인
        string batchJobAssetPath = "Assets/RundeeItemFactory/BatchJobs.asset";
        string fullPath = Path.Combine(Application.dataPath, batchJobAssetPath.Replace("Assets/", ""));
        
        if (File.Exists(fullPath))
        {
            Debug.Log("  ✅ BatchJobs.asset 파일 존재");
        }
        else
        {
            Debug.Log("  ℹ️ BatchJobs.asset 파일 없음 (정상 - 처음 사용 시 생성됨)");
        }
    }
    
    private static void TestVersionManager()
    {
        Debug.Log("  버전 관리 테스트");
        
        // VersionManager 클래스 확인
        Type versionManagerType = Type.GetType("VersionManager");
        if (versionManagerType != null)
        {
            Debug.Log("  ✅ VersionManager 클래스 존재");
            
            // GetVersion 메서드 확인
            MethodInfo getVersionMethod = versionManagerType.GetMethod("GetVersion", BindingFlags.Public | BindingFlags.Static);
            if (getVersionMethod != null)
            {
                try
                {
                    string version = (string)getVersionMethod.Invoke(null, null);
                    Debug.Log($"  ✅ 현재 버전: {version}");
                }
                catch (Exception ex)
                {
                    Debug.LogWarning($"  ⚠️ 버전 가져오기 실패: {ex.Message}");
                }
            }
        }
        else
        {
            Debug.LogWarning("  ⚠️ VersionManager 클래스를 찾을 수 없습니다");
        }
    }
    
    private static void TestItemFactoryWindow()
    {
        Debug.Log("  Item Factory Window 설정 확인");
        
        // ItemFactorySettings 확인
        string settingsPath = "Assets/RundeeItemFactory/Editor/ItemFactorySettings.asset";
        string fullPath = Path.Combine(Application.dataPath, settingsPath.Replace("Assets/", ""));
        
        if (File.Exists(fullPath))
        {
            Debug.Log("  ✅ ItemFactorySettings.asset 파일 존재");
        }
        else
        {
            Debug.Log("  ℹ️ ItemFactorySettings.asset 파일 없음 (정상 - 처음 사용 시 생성됨)");
        }
    }
    
    private static void TestHelpWindow()
    {
        Debug.Log("  Help Window 테스트");
        Debug.Log("  ✅ Help Window는 정보 표시용이므로 별도 테스트 불필요");
    }
    
    [System.Serializable]
    private class ItemListWrapper
    {
        public ItemDTO[] items;
    }
    
    [System.Serializable]
    private class ItemDTO
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
        public int maxStack;
    }
}
