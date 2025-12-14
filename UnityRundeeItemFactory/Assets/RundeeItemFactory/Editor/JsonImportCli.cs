using System;
using System.IO;
using UnityEditor;
using UnityEngine;

public static class JsonImportCli
{
    // Usage: Unity.exe -projectPath <path> -executeMethod JsonImportCli.Run --jsonPath <path> --itemType <food|drink|material|weapon|weaponcomponent|ammo>
    public static void Run()
    {
        string[] args = Environment.GetCommandLineArgs();
        string jsonPath = null;
        string itemType = null;

        for (int i = 0; i < args.Length; i++)
        {
            if (args[i] == "--jsonPath" && i + 1 < args.Length)
            {
                jsonPath = args[i + 1];
            }
            if (args[i] == "--itemType" && i + 1 < args.Length)
            {
                itemType = args[i + 1];
            }
        }

        if (string.IsNullOrEmpty(jsonPath) || string.IsNullOrEmpty(itemType))
        {
            Debug.LogError("[JsonImportCli] Usage: --jsonPath <path> --itemType <food|drink|material|weapon|weaponcomponent|ammo>");
            return;
        }

        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"[JsonImportCli] File not found: {jsonPath}");
            return;
        }

        try
        {
            switch (itemType.ToLower())
            {
                case "food":
                    ItemImporter.ImportFoodFromJsonPath(jsonPath);
                    break;
                case "drink":
                    ItemImporter.ImportDrinkFromJsonPath(jsonPath);
                    break;
                case "material":
                    ItemImporter.ImportMaterialFromJsonPath(jsonPath);
                    break;
                case "weapon":
                    ItemImporter.ImportWeaponFromJsonPath(jsonPath);
                    break;
                case "weaponcomponent":
                case "weapon_component":
                    ItemImporter.ImportWeaponComponentFromJsonPath(jsonPath);
                    break;
                case "ammo":
                    ItemImporter.ImportAmmoFromJsonPath(jsonPath);
                    break;
                default:
                    Debug.LogError($"[JsonImportCli] Unknown itemType: {itemType}");
                    return;
            }

            Debug.Log($"[JsonImportCli] Import success: {itemType} from {jsonPath}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[JsonImportCli] Import failed: {ex.Message}");
        }
    }
}


