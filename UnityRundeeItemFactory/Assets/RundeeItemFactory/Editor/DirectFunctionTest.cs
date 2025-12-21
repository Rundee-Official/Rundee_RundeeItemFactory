using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// 각 윈도우의 기능을 직접 실행하는 테스트
/// </summary>
public static class DirectFunctionTest
{
    private static string testPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/TestOutput";
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Execute All Functions")]
    public static void ExecuteAllFunctions()
    {
        Debug.Log("========================================");
        Debug.Log("=== 각 기능 직접 실행 테스트 ===");
        Debug.Log("========================================");
        
        // Test 1: Item Preview - JSON 로드
        TestItemPreviewFunction();
        
        // Test 2: Statistics - 데이터 분석
        TestStatisticsFunction();
        
        // Test 3: Quality Check - 품질 검사
        TestQualityCheckFunction();
        
        // Test 4: Data Exporter - CSV 내보내기
        TestDataExporterFunction();
        
        // Test 5: Item Manager - 리소스 확인
        TestItemManagerFunction();
        
        // Test 6: Registry Manager - 레지스트리 확인
        TestRegistryManagerFunction();
        
        Debug.Log("========================================");
        Debug.Log("=== 모든 기능 테스트 완료 ===");
        Debug.Log("========================================");
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Item Preview Load")]
    public static void TestItemPreviewFunction()
    {
        Debug.Log("\n[Test] Item Preview - JSON 파일 로드");
        string jsonPath = Path.Combine(testPath, "items_food.json");
        
        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"파일 없음: {jsonPath}");
            return;
        }
        
        string jsonContent = File.ReadAllText(jsonPath);
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        Debug.Log($"✅ {wrapper.items.Length}개 아이템 로드 성공");
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Statistics Analysis")]
    public static void TestStatisticsFunction()
    {
        Debug.Log("\n[Test] Statistics Dashboard - 데이터 분석");
        string jsonPath = Path.Combine(testPath, "items_weapon.json");
        
        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"파일 없음: {jsonPath}");
            return;
        }
        
        string jsonContent = File.ReadAllText(jsonPath);
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        var rarityDist = wrapper.items.GroupBy(i => i.rarity ?? "Unknown")
            .ToDictionary(g => g.Key, g => g.Count());
        
        Debug.Log($"✅ 통계 분석 완료: {wrapper.items.Length}개 아이템");
        foreach (var kvp in rarityDist)
        {
            Debug.Log($"   - {kvp.Key}: {kvp.Value}개");
        }
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Quality Check")]
    public static void TestQualityCheckFunction()
    {
        Debug.Log("\n[Test] Quality Check - 품질 검사");
        string jsonPath = Path.Combine(testPath, "items_ammo.json");
        
        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"파일 없음: {jsonPath}");
            return;
        }
        
        string jsonContent = File.ReadAllText(jsonPath);
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        int valid = wrapper.items.Count(i => !string.IsNullOrEmpty(i.id) && !string.IsNullOrEmpty(i.displayName));
        int invalid = wrapper.items.Length - valid;
        
        Debug.Log($"✅ 품질 검사 완료: 유효 {valid}개, 무효 {invalid}개");
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Data Exporter")]
    public static void TestDataExporterFunction()
    {
        Debug.Log("\n[Test] Data Exporter - CSV 내보내기");
        string jsonPath = Path.Combine(testPath, "items_material.json");
        
        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"파일 없음: {jsonPath}");
            return;
        }
        
        string jsonContent = File.ReadAllText(jsonPath);
        if (jsonContent.Trim().StartsWith("["))
        {
            jsonContent = "{ \"items\": " + jsonContent + " }";
        }
        
        var wrapper = JsonUtility.FromJson<ItemListWrapper>(jsonContent);
        var itemDataList = wrapper.items.Select(i => new DataExporter.ItemData
        {
            id = i.id ?? "",
            displayName = i.displayName ?? "",
            category = i.category ?? "",
            rarity = i.rarity ?? "",
            description = i.description ?? ""
        }).ToList();
        
        string csvPath = Path.Combine(Application.temporaryCachePath, "test_export.csv");
        DataExporter.ExportToCSV(csvPath, itemDataList);
        
        var imported = DataExporter.ImportFromCSV(csvPath);
        Debug.Log($"✅ CSV 내보내기/가져오기 성공: {imported.Count}개");
        
        File.Delete(csvPath);
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Item Manager")]
    public static void TestItemManagerFunction()
    {
        Debug.Log("\n[Test] Item Manager - 리소스 확인");
        
        string[] paths = {
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
        
        int total = 0;
        foreach (string path in paths)
        {
            string fullPath = Path.Combine(Application.dataPath, path.Replace("Assets/", ""));
            if (Directory.Exists(fullPath))
            {
                int count = Directory.GetFiles(fullPath, "*.asset").Length;
                total += count;
                Debug.Log($"   ✅ {Path.GetFileName(path)}: {count}개");
            }
        }
        
        Debug.Log($"✅ 총 {total}개 리소스 확인");
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Direct Test - Registry Manager")]
    public static void TestRegistryManagerFunction()
    {
        Debug.Log("\n[Test] Registry Manager - 레지스트리 확인");
        
        string registryPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/Registry";
        if (Directory.Exists(registryPath))
        {
            string[] files = Directory.GetFiles(registryPath, "*.txt");
            Debug.Log($"✅ 레지스트리 파일 {files.Length}개 발견");
            foreach (string file in files)
            {
                int lineCount = File.ReadAllLines(file).Length;
                Debug.Log($"   - {Path.GetFileName(file)}: {lineCount}개 ID");
            }
        }
        else
        {
            Debug.LogWarning($"⚠️ 레지스트리 폴더 없음: {registryPath}");
        }
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
    }
}
