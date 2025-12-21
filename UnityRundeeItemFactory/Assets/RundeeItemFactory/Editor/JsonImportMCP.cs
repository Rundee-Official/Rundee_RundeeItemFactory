using System;
using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// MCP를 통한 Unity 임포트를 위한 헬퍼 클래스
/// </summary>
public static class JsonImportMCP
{
    /// <summary>
    /// MCP를 통해 JSON 파일을 Unity로 임포트합니다.
    /// </summary>
    /// <param name="jsonPath">임포트할 JSON 파일의 전체 경로</param>
    /// <param name="itemType">아이템 타입 (food, drink, material, weapon, weaponcomponent, ammo, armor, clothing)</param>
    public static void ImportFromPath(string jsonPath, string itemType)
    {
        if (string.IsNullOrEmpty(jsonPath) || !File.Exists(jsonPath))
        {
            Debug.LogError($"[JsonImportMCP] File not found: {jsonPath}");
            return;
        }

        try
        {
            switch (itemType.ToLower())
            {
                case "food":
                    ItemImporter.ImportFoodFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported food items from {jsonPath}");
                    break;
                case "drink":
                    ItemImporter.ImportDrinkFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported drink items from {jsonPath}");
                    break;
                case "medicine":
                    ItemImporter.ImportMedicineFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported medicine items from {jsonPath}");
                    break;
                case "material":
                    ItemImporter.ImportMaterialFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported material items from {jsonPath}");
                    break;
                case "weapon":
                    ItemImporter.ImportWeaponFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported weapon items from {jsonPath}");
                    break;
                case "weaponcomponent":
                case "weapon_component":
                    ItemImporter.ImportWeaponComponentFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported weapon component items from {jsonPath}");
                    break;
                case "ammo":
                    ItemImporter.ImportAmmoFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported ammo items from {jsonPath}");
                    break;
                case "armor":
                    ItemImporter.ImportArmorFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported armor items from {jsonPath}");
                    break;
                case "clothing":
                    ItemImporter.ImportClothingFromJsonPath(jsonPath);
                    Debug.Log($"[JsonImportMCP] Successfully imported clothing items from {jsonPath}");
                    break;
                default:
                    Debug.LogError($"[JsonImportMCP] Unknown itemType: {itemType}");
                    return;
            }
        }
        catch (Exception ex)
        {
            Debug.LogError($"[JsonImportMCP] Import failed: {ex.Message}\n{ex.StackTrace}");
        }
    }

    /// <summary>
    /// MCP를 통해 모든 테스트 JSON 파일을 Unity로 임포트합니다.
    /// </summary>
    [MenuItem("Tools/Rundee/Item Factory/Import All Test Files")]
    public static void ImportAllTestFiles()
    {
        string testOutputDir = "D:/_VisualStudioProjects/_Rundee_RundeeItemFactory/TestOutput";
        
        var imports = new[]
        {
            new { type = "food", path = $"{testOutputDir}/items_food.json" },
            new { type = "drink", path = $"{testOutputDir}/items_drink.json" },
            new { type = "medicine", path = $"{testOutputDir}/items_medicine.json" },
            new { type = "material", path = $"{testOutputDir}/items_material.json" },
            new { type = "weapon", path = $"{testOutputDir}/items_weapon.json" },
            new { type = "weaponcomponent", path = $"{testOutputDir}/items_weaponcomponent.json" },
            new { type = "ammo", path = $"{testOutputDir}/items_ammo.json" },
            new { type = "armor", path = $"{testOutputDir}/items_armor.json" },
            new { type = "clothing", path = $"{testOutputDir}/items_clothing.json" }
        };
        
        int successCount = 0;
        int failCount = 0;
        
        Debug.Log("[JsonImportMCP] Starting import of all test files...");
        
        foreach (var import in imports)
        {
            try
            {
                Debug.Log($"[JsonImportMCP] Importing {import.type} from {import.path}...");
                ImportFromPath(import.path, import.type);
                successCount++;
                Debug.Log($"[JsonImportMCP] Successfully imported {import.type}");
            }
            catch (Exception ex)
            {
                failCount++;
                Debug.LogError($"[JsonImportMCP] Failed to import {import.type}: {ex.Message}");
            }
        }
        
        Debug.Log($"[JsonImportMCP] Import complete. Success: {successCount}, Failed: {failCount}");
        
        if (failCount == 0)
        {
            Debug.Log("[JsonImportMCP] All imports completed successfully!");
        }
        else
        {
            Debug.LogWarning($"[JsonImportMCP] Some imports failed. Check the console for details.");
        }
    }
}

