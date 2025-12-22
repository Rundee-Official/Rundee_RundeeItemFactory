using System;
using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Dynamic item importer for profile-based items
/// </summary>
public static class DynamicItemImporter
{
    /// <summary>
    /// Import items from JSON file using a profile
    /// </summary>
    /// <param name="jsonPath">Path to JSON file</param>
    /// <param name="profileId">Profile ID to use (if empty, tries to detect from JSON)</param>
    /// <param name="itemTypeName">Item type name (e.g., "Food", "Weapon")</param>
    /// <returns>Number of items imported</returns>
    public static int ImportFromJson(string jsonPath, string profileId, string itemTypeName)
    {
        if (!File.Exists(jsonPath))
        {
            Debug.LogError($"[DynamicItemImporter] JSON file not found: {jsonPath}");
            return 0;
        }
        
        try
        {
            // Load JSON (simple parsing for now - assumes JSON array)
            string jsonContent = File.ReadAllText(jsonPath);
            
            // Simple JSON array parsing (Unity JsonUtility doesn't support arrays directly)
            // Remove array brackets and split items
            jsonContent = jsonContent.Trim();
            if (jsonContent.StartsWith("["))
            {
                jsonContent = jsonContent.Substring(1);
            }
            if (jsonContent.EndsWith("]"))
            {
                jsonContent = jsonContent.Substring(0, jsonContent.Length - 1);
            }
            
            // Split by },{ pattern
            string[] itemStrings = jsonContent.Split(new string[] { "}," }, StringSplitOptions.None);
            
            if (itemStrings == null || itemStrings.Length == 0)
            {
                Debug.LogWarning($"[DynamicItemImporter] No items found in JSON file: {jsonPath}");
                return 0;
            }
            
            // Determine output directory
            string outputDir = $"Assets/Resources/RundeeItemFactory/{itemTypeName}Items";
            if (!Directory.Exists(outputDir))
            {
                Directory.CreateDirectory(outputDir);
            }
            
            int importedCount = 0;
            
            // Parse each item
            for (int i = 0; i < itemStrings.Length; i++)
            {
                string itemStr = itemStrings[i].Trim();
                if (string.IsNullOrEmpty(itemStr))
                    continue;
                
                // Fix last item (might not have comma)
                if (!itemStr.EndsWith("}"))
                {
                    itemStr += "}";
                }
                if (!itemStr.StartsWith("{"))
                {
                    itemStr = "{" + itemStr;
                }
                
                // Parse JSON object manually (simple approach)
                Dictionary<string, string> itemFields = ParseJsonObject(itemStr);
                
                if (!itemFields.ContainsKey("id") || string.IsNullOrEmpty(itemFields["id"]))
                {
                    Debug.LogWarning("[DynamicItemImporter] Skipping item with no ID");
                    continue;
                }
                
                string itemId = itemFields["id"];
                
                // Create ScriptableObject instance
                DynamicItemDataSO itemSO = ScriptableObject.CreateInstance<DynamicItemDataSO>();
                itemSO.profileId = profileId;
                itemSO.itemTypeName = itemTypeName;
                itemSO.fieldValues = itemFields;
                
                // Save asset
                string assetPath = $"{outputDir}/{itemId}.asset";
                AssetDatabase.CreateAsset(itemSO, assetPath);
                importedCount++;
            }
            
            AssetDatabase.SaveAssets();
            AssetDatabase.Refresh();
            
            Debug.Log($"[DynamicItemImporter] Successfully imported {importedCount} {itemTypeName} items from {jsonPath}");
            return importedCount;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[DynamicItemImporter] Failed to import items: {ex.Message}\n{ex.StackTrace}");
            return 0;
        }
    }
    
    /// <summary>
    /// Simple JSON object parser (extracts key-value pairs)
    /// </summary>
    private static Dictionary<string, string> ParseJsonObject(string jsonStr)
    {
        Dictionary<string, string> fields = new Dictionary<string, string>();
        
        // Remove outer braces
        jsonStr = jsonStr.Trim().TrimStart('{').TrimEnd('}').Trim();
        
        // Split by comma (careful with nested objects/arrays)
        List<string> pairs = new List<string>();
        int depth = 0;
        int start = 0;
        bool inString = false;
        
        for (int i = 0; i < jsonStr.Length; i++)
        {
            char c = jsonStr[i];
            
            if (c == '"' && (i == 0 || jsonStr[i - 1] != '\\'))
            {
                inString = !inString;
            }
            else if (!inString)
            {
                if (c == '{' || c == '[')
                    depth++;
                else if (c == '}' || c == ']')
                    depth--;
                else if (c == ',' && depth == 0)
                {
                    pairs.Add(jsonStr.Substring(start, i - start));
                    start = i + 1;
                }
            }
        }
        if (start < jsonStr.Length)
        {
            pairs.Add(jsonStr.Substring(start));
        }
        
        // Parse each key-value pair
        foreach (string pair in pairs)
        {
            int colonIndex = pair.IndexOf(':');
            if (colonIndex > 0)
            {
                string key = pair.Substring(0, colonIndex).Trim().Trim('"');
                string value = pair.Substring(colonIndex + 1).Trim();
                
                // Remove quotes from string values
                if (value.StartsWith("\"") && value.EndsWith("\""))
                {
                    value = value.Substring(1, value.Length - 2);
                }
                
                fields[key] = value;
            }
        }
        
        return fields;
    }
    
    /// <summary>
    /// Load profile from JSON file (Unity side)
    /// </summary>
    public static ItemProfile LoadProfileFromJson(string profilePath)
    {
        if (!File.Exists(profilePath))
        {
            Debug.LogError($"[DynamicItemImporter] Profile file not found: {profilePath}");
            return null;
        }
        
        try
        {
            string jsonContent = File.ReadAllText(profilePath);
            ItemProfile profile = JsonUtility.FromJson<ItemProfile>(jsonContent);
            return profile;
        }
        catch (Exception ex)
        {
            Debug.LogError($"[DynamicItemImporter] Failed to load profile: {ex.Message}");
            return null;
        }
    }
}

