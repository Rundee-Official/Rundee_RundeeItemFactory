using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// 빠른 Unity 기능 테스트
/// </summary>
public static class QuickUnityTest
{
    private static string testPath = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/Deployment/TestOutput";
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Quick Test - Item Import")]
    public static void TestItemImport()
    {
        Debug.Log("=== Item Import 빠른 테스트 ===");
        
        var files = new[]
        {
            ("food", "items_food.json"),
            ("drink", "items_drink.json"),
            ("medicine", "items_medicine.json"),
            ("material", "items_material.json"),
            ("weapon", "items_weapon.json"),
            ("weaponcomponent", "items_weaponcomponent.json"),
            ("ammo", "items_ammo.json"),
            ("armor", "items_armor.json"),
            ("clothing", "items_clothing.json")
        };
        
        int success = 0;
        int failed = 0;
        
        foreach (var (type, filename) in files)
        {
            string fullPath = Path.Combine(testPath, filename);
            if (!File.Exists(fullPath))
            {
                Debug.LogWarning($"⚠️ 파일 없음: {fullPath}");
                failed++;
                continue;
            }
            
            try
            {
                Debug.Log($"임포트 중: {type} ({filename})");
                
                switch (type)
                {
                    case "food":
                        ItemImporter.ImportFoodFromJsonPath(fullPath);
                        break;
                    case "drink":
                        ItemImporter.ImportDrinkFromJsonPath(fullPath);
                        break;
                    case "medicine":
                        ItemImporter.ImportMedicineFromJsonPath(fullPath);
                        break;
                    case "material":
                        ItemImporter.ImportMaterialFromJsonPath(fullPath);
                        break;
                    case "weapon":
                        ItemImporter.ImportWeaponFromJsonPath(fullPath);
                        break;
                    case "weaponcomponent":
                        ItemImporter.ImportWeaponComponentFromJsonPath(fullPath);
                        break;
                    case "ammo":
                        ItemImporter.ImportAmmoFromJsonPath(fullPath);
                        break;
                    case "armor":
                        ItemImporter.ImportArmorFromJsonPath(fullPath);
                        break;
                    case "clothing":
                        ItemImporter.ImportClothingFromJsonPath(fullPath);
                        break;
                }
                
                Debug.Log($"✅ {type} 임포트 성공");
                success++;
            }
            catch (System.Exception ex)
            {
                Debug.LogError($"❌ {type} 임포트 실패: {ex.Message}");
                failed++;
            }
        }
        
        Debug.Log($"\n=== 테스트 완료 ===");
        Debug.Log($"✅ 성공: {success}개");
        Debug.Log($"❌ 실패: {failed}개");
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Test/Quick Test - Check Resources")]
    public static void CheckResources()
    {
        Debug.Log("=== 리소스 확인 테스트 ===");
        
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
                int count = Directory.GetFiles(fullPath, "*.asset", SearchOption.TopDirectoryOnly).Length;
                total += count;
                Debug.Log($"✅ {path}: {count}개");
            }
            else
            {
                Debug.LogWarning($"⚠️ {path}: 폴더 없음");
            }
        }
        
        Debug.Log($"\n총 {total}개의 아이템 리소스 확인 완료");
    }
}
