using System;
using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Unity 기능 전체 테스트를 위한 테스트 클래스
/// </summary>
public static class UnityFunctionalityTest
{
    private static string testOutputPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/TestOutput";
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Run All Functionality Tests", false, 10000)]
    public static void RunAllTests()
    {
        Debug.Log("=== Unity 기능 전체 테스트 시작 ===");
        int passed = 0;
        int failed = 0;
        
        // Test 1: Item Import
        Debug.Log("\n[Test 1] Item Import 테스트");
        try
        {
            TestItemImport();
            Debug.Log("✅ Item Import 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Import 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // Test 2: Item Preview (JSON 로드)
        Debug.Log("\n[Test 2] Item Preview (JSON 로드) 테스트");
        try
        {
            TestItemPreviewLoad();
            Debug.Log("✅ Item Preview 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Preview 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // Test 3: Statistics Dashboard (데이터 분석)
        Debug.Log("\n[Test 3] Statistics Dashboard (데이터 분석) 테스트");
        try
        {
            TestStatisticsAnalysis();
            Debug.Log("✅ Statistics Dashboard 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Statistics Dashboard 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // Test 4: Quality Check (품질 검사)
        Debug.Log("\n[Test 4] Quality Check (품질 검사) 테스트");
        try
        {
            TestQualityCheck();
            Debug.Log("✅ Quality Check 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Quality Check 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // Test 5: Data Exporter (데이터 내보내기)
        Debug.Log("\n[Test 5] Data Exporter (데이터 내보내기) 테스트");
        try
        {
            TestDataExport();
            Debug.Log("✅ Data Exporter 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Data Exporter 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // Test 6: Item Manager (아이템 리소스 확인)
        Debug.Log("\n[Test 6] Item Manager (아이템 리소스 확인) 테스트");
        try
        {
            TestItemManager();
            Debug.Log("✅ Item Manager 테스트 통과");
            passed++;
        }
        catch (Exception ex)
        {
            Debug.LogError($"❌ Item Manager 테스트 실패: {ex.Message}");
            failed++;
        }
        
        // 최종 결과
        Debug.Log($"\n=== 테스트 완료 ===");
        Debug.Log($"✅ 통과: {passed}개");
        Debug.Log($"❌ 실패: {failed}개");
        Debug.Log($"총 테스트: {passed + failed}개");
        
        if (failed == 0)
        {
            Debug.Log("🎉 모든 테스트가 성공적으로 완료되었습니다!");
        }
        else
        {
            Debug.LogWarning("⚠️ 일부 테스트가 실패했습니다. 위의 에러 메시지를 확인하세요.");
        }
    }
    
    private static void TestItemImport()
    {
        var testFiles = new[]
        {
            new { type = "food", path = Path.Combine(testOutputPath, "items_food.json") },
            new { type = "drink", path = Path.Combine(testOutputPath, "items_drink.json") },
            new { type = "medicine", path = Path.Combine(testOutputPath, "items_medicine.json") },
            new { type = "material", path = Path.Combine(testOutputPath, "items_material.json") },
            new { type = "weapon", path = Path.Combine(testOutputPath, "items_weapon.json") },
            new { type = "weaponcomponent", path = Path.Combine(testOutputPath, "items_weaponcomponent.json") },
            new { type = "ammo", path = Path.Combine(testOutputPath, "items_ammo.json") },
            new { type = "armor", path = Path.Combine(testOutputPath, "items_armor.json") },
            new { type = "clothing", path = Path.Combine(testOutputPath, "items_clothing.json") }
        };
        
        foreach (var testFile in testFiles)
        {
            if (!File.Exists(testFile.path))
            {
                throw new FileNotFoundException($"테스트 파일이 없습니다: {testFile.path}");
            }
            
            Debug.Log($"  - {testFile.type} 임포트 중... ({testFile.path})");
            
            switch (testFile.type)
            {
                case "food":
                    ItemImporter.ImportFoodFromJsonPath(testFile.path);
                    break;
                case "drink":
                    ItemImporter.ImportDrinkFromJsonPath(testFile.path);
                    break;
                case "medicine":
                    ItemImporter.ImportMedicineFromJsonPath(testFile.path);
                    break;
                case "material":
                    ItemImporter.ImportMaterialFromJsonPath(testFile.path);
                    break;
                case "weapon":
                    ItemImporter.ImportWeaponFromJsonPath(testFile.path);
                    break;
                case "weaponcomponent":
                    ItemImporter.ImportWeaponComponentFromJsonPath(testFile.path);
                    break;
                case "ammo":
                    ItemImporter.ImportAmmoFromJsonPath(testFile.path);
                    break;
                case "armor":
                    ItemImporter.ImportArmorFromJsonPath(testFile.path);
                    break;
                case "clothing":
                    ItemImporter.ImportClothingFromJsonPath(testFile.path);
                    break;
            }
            
            Debug.Log($"  ✅ {testFile.type} 임포트 완료");
        }
        
        Debug.Log("  모든 타입 임포트 성공");
    }
    
    private static void TestItemPreviewLoad()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_food.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  JSON 파일 로드 테스트: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (string.IsNullOrEmpty(jsonContent))
        {
            throw new Exception("JSON 파일이 비어있습니다.");
        }
        
        // JSON 배열을 객체로 래핑 (Unity JsonUtility 호환성)
        if (jsonContent.TrimStart().StartsWith("["))
        {
            jsonContent = "{\"items\":" + jsonContent + "}";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        Debug.Log($"  ✅ {wrapper.items.Length}개의 아이템 로드 성공");
    }
    
    private static void TestStatisticsAnalysis()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_weapon.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  통계 분석 테스트: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (string.IsNullOrEmpty(jsonContent))
        {
            throw new Exception("JSON 파일이 비어있습니다.");
        }
        
        // JSON 배열을 객체로 래핑
        if (jsonContent.TrimStart().StartsWith("["))
        {
            jsonContent = "{\"items\":" + jsonContent + "}";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // 기본 통계 계산
        int totalItems = wrapper.items.Length;
        var rarityCount = new System.Collections.Generic.Dictionary<string, int>();
        
        foreach (var item in wrapper.items)
        {
            string rarity = item.rarity ?? "Unknown";
            if (!rarityCount.ContainsKey(rarity))
            {
                rarityCount[rarity] = 0;
            }
            rarityCount[rarity]++;
        }
        
        Debug.Log($"  ✅ 통계 분석 완료:");
        Debug.Log($"     - 총 아이템 수: {totalItems}");
        foreach (var kvp in rarityCount)
        {
            Debug.Log($"     - {kvp.Key}: {kvp.Value}개");
        }
    }
    
    private static void TestQualityCheck()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_ammo.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  품질 검사 테스트: {testJsonPath}");
        string jsonContent = File.ReadAllText(testJsonPath);
        
        if (string.IsNullOrEmpty(jsonContent))
        {
            throw new Exception("JSON 파일이 비어있습니다.");
        }
        
        // JSON 배열을 객체로 래핑
        if (jsonContent.TrimStart().StartsWith("["))
        {
            jsonContent = "{\"items\":" + jsonContent + "}";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // 기본 유효성 검사
        int validCount = 0;
        int invalidCount = 0;
        
        foreach (var item in wrapper.items)
        {
            bool isValid = !string.IsNullOrEmpty(item.id) && 
                          !string.IsNullOrEmpty(item.displayName) &&
                          !string.IsNullOrEmpty(item.category);
            
            if (isValid)
            {
                validCount++;
            }
            else
            {
                invalidCount++;
                Debug.LogWarning($"  ⚠️ 유효하지 않은 아이템: {item.id ?? "NULL"}");
            }
        }
        
        Debug.Log($"  ✅ 품질 검사 완료:");
        Debug.Log($"     - 유효한 아이템: {validCount}개");
        Debug.Log($"     - 무효한 아이템: {invalidCount}개");
    }
    
    private static void TestDataExport()
    {
        string testJsonPath = Path.Combine(testOutputPath, "items_material.json");
        if (!File.Exists(testJsonPath))
        {
            throw new FileNotFoundException($"테스트 파일이 없습니다: {testJsonPath}");
        }
        
        Debug.Log($"  데이터 내보내기 테스트: {testJsonPath}");
        
        // JSON 로드
        string jsonContent = File.ReadAllText(testJsonPath);
        if (jsonContent.TrimStart().StartsWith("["))
        {
            jsonContent = "{\"items\":" + jsonContent + "}";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        
        if (wrapper == null || wrapper.items == null)
        {
            throw new Exception("JSON 파싱 실패");
        }
        
        // ItemData 리스트로 변환 (DataExporter.ItemData 사용)
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
        
        // CSV 내보내기 테스트
        string tempCsvPath = Path.Combine(Application.temporaryCachePath, "test_export.csv");
        DataExporter.ExportToCSV(tempCsvPath, itemDataList);
        
        if (!File.Exists(tempCsvPath))
        {
            throw new Exception("CSV 파일 생성 실패");
        }
        
        Debug.Log($"  ✅ CSV 내보내기 성공: {tempCsvPath}");
        
        // CSV 다시 읽기 테스트
        var importedItems = DataExporter.ImportFromCSV(tempCsvPath);
        Debug.Log($"  ✅ CSV 가져오기 성공: {importedItems.Count}개 아이템");
        
        // 임시 파일 정리
        File.Delete(tempCsvPath);
    }
    
    private static void TestItemManager()
    {
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
                string[] assetFiles = Directory.GetFiles(fullPath, "*.asset", SearchOption.TopDirectoryOnly);
                totalItems += assetFiles.Length;
                Debug.Log($"  ✅ {path}: {assetFiles.Length}개");
            }
            else
            {
                Debug.LogWarning($"  ⚠️ {path}: 폴더 없음");
            }
        }
        
        Debug.Log($"  ✅ 총 {totalItems}개의 아이템 리소스 확인 완료");
    }
    
    [System.Serializable]
    private class ItemListWrapper
    {
        public TestItemData[] items;
    }
    
    [System.Serializable]
    private class TestItemData
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
    }
}
