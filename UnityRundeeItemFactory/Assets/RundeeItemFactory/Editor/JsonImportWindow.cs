// ===============================
// Project Name: RundeeItemFactory
// File Name: JsonImportWindow.cs
// Author: Haneul Lee (Rundee)
// Created Date: 2025-12-16
// Description: Simple one-stop JSON importer window that routes to the existing ItemImporter methods for each item type.
// ===============================
// Copyright (c) 2025 Haneul Lee. All rights reserved.
// ===============================

using System;
using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Simple one-stop JSON importer that routes to the existing ItemImporter methods for each item type.
/// </summary>
public class JsonImportWindow : EditorWindow
{
    private ItemType _selectedType = ItemType.Food;
    private string _jsonPath = string.Empty;

    [MenuItem("Tools/Rundee/Item Factory/JSON Importer")]
    public static void ShowWindow()
    {
        var window = GetWindow<JsonImportWindow>("JSON Importer");
        window.minSize = new Vector2(380, 160);
    }

    private void OnGUI()
    {
        GUILayout.Label("Import item JSON into ScriptableObjects", EditorStyles.boldLabel);
        _selectedType = (ItemType)EditorGUILayout.EnumPopup("Item type", _selectedType);

        EditorGUILayout.Space(6f);
        DrawPathPicker();

        EditorGUILayout.Space(12f);
        using (new EditorGUI.DisabledScope(string.IsNullOrEmpty(_jsonPath) || !File.Exists(_jsonPath)))
        {
            if (GUILayout.Button("Import Selected JSON", GUILayout.Height(30)))
            {
                ImportSelected();
            }
        }

        if (!string.IsNullOrEmpty(_jsonPath) && !File.Exists(_jsonPath))
        {
            EditorGUILayout.HelpBox("File not found at the specified path.", MessageType.Warning);
        }
    }

    private void DrawPathPicker()
    {
        using (new EditorGUILayout.HorizontalScope())
        {
            EditorGUILayout.LabelField("JSON file", GUILayout.Width(70));
            _jsonPath = EditorGUILayout.TextField(_jsonPath);
            if (GUILayout.Button("Browse", GUILayout.Width(70)))
            {
                string initialDir = string.IsNullOrEmpty(_jsonPath)
                    ? Application.dataPath
                    : Path.GetDirectoryName(_jsonPath);
                string picked = EditorUtility.OpenFilePanel("Select item JSON", initialDir, "json");
                if (!string.IsNullOrEmpty(picked))
                {
                    _jsonPath = picked;
                }
            }
        }
    }

    private void ImportSelected()
    {
        try
        {
            switch (_selectedType)
            {
                case ItemType.Food:
                    ItemImporter.ImportFoodFromJsonPath(_jsonPath);
                    break;
                case ItemType.Drink:
                    ItemImporter.ImportDrinkFromJsonPath(_jsonPath);
                    break;
                case ItemType.Material:
                    ItemImporter.ImportMaterialFromJsonPath(_jsonPath);
                    break;
                case ItemType.Weapon:
                    ItemImporter.ImportWeaponFromJsonPath(_jsonPath);
                    break;
                case ItemType.WeaponComponent:
                    ItemImporter.ImportWeaponComponentFromJsonPath(_jsonPath);
                    break;
                case ItemType.Ammo:
                    ItemImporter.ImportAmmoFromJsonPath(_jsonPath);
                    break;
                default:
                    Debug.LogWarning("[JSON Importer] Unknown item type.");
                    return;
            }

            ShowNotification(new GUIContent("Import complete. Check console for details."));
        }
        catch (Exception ex)
        {
            Debug.LogError($"[JSON Importer] Failed: {ex.Message}");
        }
    }
}

