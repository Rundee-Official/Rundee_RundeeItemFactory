// ===============================
// Project Name: RundeeItemFactory
// File Name: ItemFactoryDebugTools.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Debug and test tools for Item Factory. Accessible via Tools > Rundee > Item Factory > [Debug] menu.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Debug and test tools for Item Factory.
/// Accessible via Tools > Rundee > Item Factory > [Debug] menu.
/// </summary>
public static class ItemFactoryDebugTools
{
    /// <summary>
    /// Gets the repository root directory.
    /// Application.dataPath = UnityRundeeItemFactory/Assets
    /// Project root = UnityRundeeItemFactory (one level up)
    /// Repository root = _Rundee_RundeeItemFactory (two levels up)
    /// </summary>
    private static string GetRepoRoot()
    {
        string projectRoot = Path.GetFullPath(Path.Combine(Application.dataPath, ".."));
        string repoRoot = Path.GetFullPath(Path.Combine(projectRoot, ".."));
        return repoRoot;
    }

    [MenuItem("Tools/Rundee/Item Factory/[Debug] Import All Test Items", false, 100)]
    public static void ImportAllTestItems()
    {
        string repoRoot = GetRepoRoot();
        string testDir = Path.Combine(repoRoot, "FinalTest");
        
        Debug.Log($"[ItemFactoryDebugTools] Starting import from: {testDir}");
        
        int successCount = 0;
        int totalCount = 0;
        
        // Food
        string foodPath = Path.Combine(testDir, "final_test_food.json");
        if (File.Exists(foodPath)) {
            try {
                ItemImporter.ImportFoodFromJsonPath(foodPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ Food imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ Food failed: {e.Message}");
            }
            totalCount++;
        }
        
        // Drink
        string drinkPath = Path.Combine(testDir, "final_test_drink.json");
        if (File.Exists(drinkPath)) {
            try {
                ItemImporter.ImportDrinkFromJsonPath(drinkPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ Drink imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ Drink failed: {e.Message}");
            }
            totalCount++;
        }
        
        // Material
        string materialPath = Path.Combine(testDir, "final_test_material.json");
        if (File.Exists(materialPath)) {
            try {
                ItemImporter.ImportMaterialFromJsonPath(materialPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ Material imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ Material failed: {e.Message}");
            }
            totalCount++;
        }
        
        // Weapon
        string weaponPath = Path.Combine(testDir, "final_test_weapon.json");
        if (File.Exists(weaponPath)) {
            try {
                ItemImporter.ImportWeaponFromJsonPath(weaponPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ Weapon imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ Weapon failed: {e.Message}");
            }
            totalCount++;
        }
        
        // WeaponComponent
        string weaponComponentPath = Path.Combine(testDir, "final_test_weaponcomponent.json");
        if (File.Exists(weaponComponentPath)) {
            try {
                ItemImporter.ImportWeaponComponentFromJsonPath(weaponComponentPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ WeaponComponent imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ WeaponComponent failed: {e.Message}");
            }
            totalCount++;
        }
        
        // Ammo
        string ammoPath = Path.Combine(testDir, "final_test_ammo.json");
        if (File.Exists(ammoPath)) {
            try {
                ItemImporter.ImportAmmoFromJsonPath(ammoPath);
                Debug.Log($"[ItemFactoryDebugTools] ✓ Ammo imported");
                successCount++;
            } catch (Exception e) {
                Debug.LogError($"[ItemFactoryDebugTools] ✗ Ammo failed: {e.Message}");
            }
            totalCount++;
        }
        
        AssetDatabase.Refresh();
        Debug.Log($"[ItemFactoryDebugTools] ========================================");
        Debug.Log($"[ItemFactoryDebugTools] Results: {successCount} / {totalCount} successful");
        Debug.Log($"[ItemFactoryDebugTools] ========================================");
        EditorUtility.DisplayDialog("Import All Test Items", $"Import completed: {successCount} / {totalCount} successful", "OK");
    }

    /// <summary>
    /// Sets up runtime test by finding or creating ItemDatabase and entering Play Mode.
    /// </summary>
    [MenuItem("Tools/Rundee/Item Factory/[Debug] Setup Runtime Test", false, 101)]
    public static void SetupRuntimeTest()
    {
        // Find ItemDatabase component in the scene
        ItemDatabase itemDatabase = UnityEngine.Object.FindFirstObjectByType<ItemDatabase>();
        
        if (itemDatabase == null)
        {
            // Create ItemDatabase if it doesn't exist
            GameObject dbObject = new GameObject("ItemDatabase");
            itemDatabase = dbObject.AddComponent<ItemDatabase>();
            Debug.Log("[ItemFactoryDebugTools] ItemDatabase GameObject created successfully");
        }
        else
        {
            Debug.Log("[ItemFactoryDebugTools] Existing ItemDatabase found");
        }
        
        // Enter Play Mode
        if (!Application.isPlaying)
        {
            Debug.Log("[ItemFactoryDebugTools] Starting Play Mode...");
            EditorApplication.isPlaying = true;
        }
        else
        {
            Debug.Log("[ItemFactoryDebugTools] Already in Play Mode. Verifying ItemDatabase load...");
            itemDatabase.LoadAllItems();
        }
    }
    
    /// <summary>
    /// Tests ItemDatabase in Play Mode. Loads all items and displays summary.
    /// </summary>
    [MenuItem("Tools/Rundee/Item Factory/[Debug] Test ItemDatabase (Play Mode Only)", false, 102)]
    public static void TestItemDatabase()
    {
        if (!Application.isPlaying)
        {
            Debug.LogWarning("[ItemFactoryDebugTools] This can only be executed in Play Mode.");
            EditorUtility.DisplayDialog("Warning", "This can only be executed in Play Mode.", "OK");
            return;
        }
        
        ItemDatabase itemDatabase = UnityEngine.Object.FindFirstObjectByType<ItemDatabase>();
        
        if (itemDatabase == null)
        {
            Debug.LogError("[ItemFactoryDebugTools] ItemDatabase not found.");
            return;
        }
        
        Debug.Log("=== ItemDatabase Runtime Test Started ===");
        itemDatabase.LoadAllItems();
        
        // Summary of results
        int totalItems = itemDatabase.foodItems.Length +
                        itemDatabase.drinkItems.Length +
                        itemDatabase.materialItems.Length +
                        itemDatabase.weaponItems.Length +
                        itemDatabase.weaponComponentItems.Length +
                        itemDatabase.ammoItems.Length;
        
        Debug.Log($"[ItemFactoryDebugTools] Total loaded items: {totalItems}");
        Debug.Log($"[ItemFactoryDebugTools] Food: {itemDatabase.foodItems.Length}");
        Debug.Log($"[ItemFactoryDebugTools] Drink: {itemDatabase.drinkItems.Length}");
        Debug.Log($"[ItemFactoryDebugTools] Material: {itemDatabase.materialItems.Length}");
        Debug.Log($"[ItemFactoryDebugTools] Weapon: {itemDatabase.weaponItems.Length}");
        Debug.Log($"[ItemFactoryDebugTools] WeaponComponent: {itemDatabase.weaponComponentItems.Length}");
        Debug.Log($"[ItemFactoryDebugTools] Ammo: {itemDatabase.ammoItems.Length}");
        
        Debug.Log("=== ItemDatabase Runtime Test Completed ===");
    }
}

