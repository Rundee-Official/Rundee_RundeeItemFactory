using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// 즉시 실행 가능한 기능 테스트
/// </summary>
public static class ImmediateFunctionTest
{
    private static string testPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/TestOutput";
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Immediate - All Functions")]
    public static void TestAllImmediate()
    {
        Debug.Log("========================================");
        Debug.Log("=== 즉시 실행 기능 테스트 ===");
        Debug.Log("========================================");
        
        Test1_ItemPreview();
        Test2_Statistics();
        Test3_QualityCheck();
        Test4_DataExporter();
        Test5_ItemManager();
        Test6_Registry();
        Test7_JsonImport();
        
        Debug.Log("========================================");
        Debug.Log("=== 모든 테스트 완료 ===");
        Debug.Log("========================================");
    }
    
    private static void Test1_ItemPreview()
    {
        Debug.Log("\n[1] Item Preview - JSON 로드");
        try
        {
            string jsonPath = Path.Combine(testPath, "items_food.json");
            string json = File.ReadAllText(jsonPath);
            if (json.Trim().StartsWith("[")) json = "{ \"items\": " + json + " }";
            var w = JsonUtility.FromJson<Wrapper>(json);
            Debug.Log($"✅ {w.items.Length}개 로드");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test2_Statistics()
    {
        Debug.Log("\n[2] Statistics - 분석");
        try
        {
            string jsonPath = Path.Combine(testPath, "items_weapon.json");
            string json = File.ReadAllText(jsonPath);
            if (json.Trim().StartsWith("[")) json = "{ \"items\": " + json + " }";
            var w = JsonUtility.FromJson<Wrapper>(json);
            var dist = w.items.GroupBy(i => i.rarity ?? "Unknown").ToDictionary(g => g.Key, g => g.Count());
            Debug.Log($"✅ {w.items.Length}개 분석, 희귀도: {string.Join(", ", dist.Select(k => $"{k.Key}:{k.Value}"))}");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test3_QualityCheck()
    {
        Debug.Log("\n[3] Quality Check - 검사");
        try
        {
            string jsonPath = Path.Combine(testPath, "items_ammo.json");
            string json = File.ReadAllText(jsonPath);
            if (json.Trim().StartsWith("[")) json = "{ \"items\": " + json + " }";
            var w = JsonUtility.FromJson<Wrapper>(json);
            int valid = w.items.Count(i => !string.IsNullOrEmpty(i.id) && !string.IsNullOrEmpty(i.displayName));
            Debug.Log($"✅ {valid}/{w.items.Length}개 유효");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test4_DataExporter()
    {
        Debug.Log("\n[4] Data Exporter - CSV");
        try
        {
            string jsonPath = Path.Combine(testPath, "items_material.json");
            string json = File.ReadAllText(jsonPath);
            if (json.Trim().StartsWith("[")) json = "{ \"items\": " + json + " }";
            var w = JsonUtility.FromJson<Wrapper>(json);
            var list = w.items.Select(i => new DataExporter.ItemData { id = i.id ?? "", displayName = i.displayName ?? "", category = i.category ?? "", rarity = i.rarity ?? "", description = i.description ?? "" }).ToList();
            string csv = Path.Combine(Application.temporaryCachePath, "test.csv");
            DataExporter.ExportToCSV(csv, list);
            var imported = DataExporter.ImportFromCSV(csv);
            File.Delete(csv);
            Debug.Log($"✅ 내보내기/가져오기 {imported.Count}개");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test5_ItemManager()
    {
        Debug.Log("\n[5] Item Manager - 리소스");
        try
        {
            string[] paths = { "FoodItems", "DrinkItems", "MedicineItems", "MaterialItems", "WeaponItems", "WeaponComponentItems", "AmmoItems", "ArmorItems", "ClothingItems" };
            int total = 0;
            foreach (string p in paths)
            {
                string full = Path.Combine(Application.dataPath, "Resources", "RundeeItemFactory", p);
                if (Directory.Exists(full)) total += Directory.GetFiles(full, "*.asset").Length;
            }
            Debug.Log($"✅ 총 {total}개 리소스");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test6_Registry()
    {
        Debug.Log("\n[6] Registry Manager - 레지스트리");
        try
        {
            string regPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/Registry";
            if (Directory.Exists(regPath))
            {
                int count = Directory.GetFiles(regPath, "*.txt").Length;
                Debug.Log($"✅ {count}개 레지스트리 파일");
            }
            else Debug.LogWarning("⚠️ 레지스트리 폴더 없음");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    private static void Test7_JsonImport()
    {
        Debug.Log("\n[7] JSON Importer - 임포트");
        try
        {
            string jsonPath = Path.Combine(testPath, "items_drink.json");
            ItemImporter.ImportDrinkFromJsonPath(jsonPath);
            Debug.Log("✅ JSON 임포트 성공");
        }
        catch (Exception ex) { Debug.LogError($"❌ 실패: {ex.Message}"); }
    }
    
    [System.Serializable]
    private class Wrapper { public Item[] items; }
    [System.Serializable]
    private class Item { public string id; public string displayName; public string category; public string rarity; public string description; }
}
