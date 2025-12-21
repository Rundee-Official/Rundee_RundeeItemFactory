using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Data export/import utilities for various formats
/// </summary>
public static class DataExporter
{
    public static void ExportToCSV(string filePath, List<ItemData> items)
    {
        try
        {
            var csv = new StringBuilder();
            
            // Header
            csv.AppendLine("ID,DisplayName,Category,Rarity,Description");
            
            // Data
            foreach (var item in items)
            {
                csv.AppendLine($"{EscapeCSV(item.id)},{EscapeCSV(item.displayName)}," +
                    $"{EscapeCSV(item.category)},{EscapeCSV(item.rarity)},{EscapeCSV(item.description)}");
            }
            
            File.WriteAllText(filePath, csv.ToString());
            Debug.Log($"[DataExporter] Exported {items.Count} items to CSV: {filePath}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[DataExporter] CSV export failed: {ex.Message}");
            throw;
        }
    }
    
    public static void ExportToYAML(string filePath, List<ItemData> items)
    {
        try
        {
            var yaml = new StringBuilder();
            yaml.AppendLine("items:");
            
            foreach (var item in items)
            {
                yaml.AppendLine("  -");
                yaml.AppendLine($"    id: {item.id}");
                yaml.AppendLine($"    displayName: {item.displayName}");
                yaml.AppendLine($"    category: {item.category}");
                yaml.AppendLine($"    rarity: {item.rarity}");
                yaml.AppendLine($"    description: {item.description}");
            }
            
            File.WriteAllText(filePath, yaml.ToString());
            Debug.Log($"[DataExporter] Exported {items.Count} items to YAML: {filePath}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[DataExporter] YAML export failed: {ex.Message}");
            throw;
        }
    }
    
    public static List<ItemData> ImportFromCSV(string filePath)
    {
        var items = new List<ItemData>();
        
        try
        {
            var lines = File.ReadAllLines(filePath);
            if (lines.Length < 2)
            {
                throw new Exception("CSV file must have at least a header and one data row");
            }
            
            // Skip header
            for (int i = 1; i < lines.Length; i++)
            {
                var fields = ParseCSVLine(lines[i]);
                if (fields.Length >= 4)
                {
                    items.Add(new ItemData
                    {
                        id = fields[0],
                        displayName = fields[1],
                        category = fields[2],
                        rarity = fields[3],
                        description = fields.Length > 4 ? fields[4] : ""
                    });
                }
            }
            
            Debug.Log($"[DataExporter] Imported {items.Count} items from CSV: {filePath}");
        }
        catch (Exception ex)
        {
            Debug.LogError($"[DataExporter] CSV import failed: {ex.Message}");
            throw;
        }
        
        return items;
    }
    
    private static string EscapeCSV(string value)
    {
        if (string.IsNullOrEmpty(value))
            return "";
        
        if (value.Contains(",") || value.Contains("\"") || value.Contains("\n"))
        {
            return "\"" + value.Replace("\"", "\"\"") + "\"";
        }
        
        return value;
    }
    
    private static string[] ParseCSVLine(string line)
    {
        var fields = new List<string>();
        bool inQuotes = false;
        var currentField = new StringBuilder();
        
        foreach (char c in line)
        {
            if (c == '"')
            {
                inQuotes = !inQuotes;
            }
            else if (c == ',' && !inQuotes)
            {
                fields.Add(currentField.ToString());
                currentField.Clear();
            }
            else
            {
                currentField.Append(c);
            }
        }
        
        fields.Add(currentField.ToString());
        return fields.ToArray();
    }
    
    public class ItemData
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
    }
}

/// <summary>
/// Unity Editor window for data export/import
/// </summary>
public class DataExporterWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string jsonFilePath = "";
    private ExportFormat selectedFormat = ExportFormat.CSV;
    
    private enum ExportFormat
    {
        CSV,
        YAML,
        JSON
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Tools/Data Exporter", false, 3030)]
    public static void ShowWindow()
    {
        var window = GetWindow<DataExporterWindow>("Data Exporter");
        window.minSize = new Vector2(500, 300);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Data Exporter", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Export and import items in various formats (CSV, YAML, JSON).",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Export Section
        EditorGUILayout.LabelField("Export", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("JSON File:", GUILayout.Width(80));
        EditorGUILayout.TextField(jsonFilePath, GUILayout.ExpandWidth(true));
        if (GUILayout.Button("Browse", GUILayout.Width(80)))
        {
            string path = EditorUtility.OpenFilePanel("Select JSON File", "", "json");
            if (!string.IsNullOrEmpty(path))
            {
                jsonFilePath = path;
            }
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        selectedFormat = (ExportFormat)EditorGUILayout.EnumPopup("Export Format:", selectedFormat);
        
        if (GUILayout.Button("Export", GUILayout.Height(30)))
        {
            ExportData();
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Import Section
        EditorGUILayout.LabelField("Import", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (GUILayout.Button("Import from CSV", GUILayout.Height(30)))
        {
            ImportFromCSV();
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void ExportData()
    {
        if (string.IsNullOrEmpty(jsonFilePath) || !File.Exists(jsonFilePath))
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.FileNotFound, jsonFilePath);
            return;
        }
        
        try
        {
            string jsonContent = File.ReadAllText(jsonFilePath);
            // Unity's JsonUtility cannot parse arrays directly, so we need to wrap it
            // Check if content starts with '[' (array) or '{' (object)
            string jsonToParse = jsonContent.Trim();
            if (jsonToParse.StartsWith("["))
            {
                // It's an array, wrap it in an object
                jsonToParse = "{ \"items\": " + jsonContent + " }";
            }
            else if (!jsonToParse.StartsWith("{"))
            {
                ErrorHandler.ShowError(ErrorHandler.ErrorType.InvalidJSON);
                return;
            }
            
            var jsonData = JsonUtility.FromJson<ItemListWrapper>(jsonToParse);
            
            if (jsonData == null || jsonData.items == null)
            {
                ErrorHandler.ShowError(ErrorHandler.ErrorType.InvalidJSON);
                return;
            }
            
            var items = jsonData.items.Select(i => new DataExporter.ItemData
            {
                id = i.id,
                displayName = i.displayName,
                category = i.category,
                rarity = i.rarity,
                description = i.description
            }).ToList();
            
            string extension = selectedFormat == ExportFormat.CSV ? "csv" : 
                              selectedFormat == ExportFormat.YAML ? "yaml" : "json";
            string defaultFileName = Path.GetFileNameWithoutExtension(jsonFilePath) + $".{extension}";
            
            string path = EditorUtility.SaveFilePanel(
                "Export Data",
                "",
                defaultFileName,
                extension);
            
            if (string.IsNullOrEmpty(path))
                return;
            
            switch (selectedFormat)
            {
                case ExportFormat.CSV:
                    DataExporter.ExportToCSV(path, items);
                    break;
                case ExportFormat.YAML:
                    DataExporter.ExportToYAML(path, items);
                    break;
                case ExportFormat.JSON:
                    File.WriteAllText(path, jsonContent);
                    break;
            }
            
            ErrorHandler.ShowSuccess($"Exported {items.Count} items to {path}");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.PermissionDenied, ex.Message);
        }
    }
    
    private void ImportFromCSV()
    {
        string path = EditorUtility.OpenFilePanel("Import CSV", "", "csv");
        if (string.IsNullOrEmpty(path))
            return;
        
        try
        {
            var items = DataExporter.ImportFromCSV(path);
            
            // Convert to JSON and save
            string jsonPath = EditorUtility.SaveFilePanel(
                "Save as JSON",
                "",
                Path.GetFileNameWithoutExtension(path) + ".json",
                "json");
            
            if (string.IsNullOrEmpty(jsonPath))
                return;
            
            var wrapper = new ItemListWrapper
            {
                items = items.Select(i => new ItemDTO
                {
                    id = i.id,
                    displayName = i.displayName,
                    category = i.category,
                    rarity = i.rarity,
                    description = i.description
                }).ToArray()
            };
            
            string json = JsonUtility.ToJson(wrapper, true);
            File.WriteAllText(jsonPath, json);
            
            ErrorHandler.ShowSuccess($"Imported {items.Count} items from CSV");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.InvalidJSON, ex.Message);
        }
    }
    
    [Serializable]
    private class ItemListWrapper
    {
        public ItemDTO[] items;
    }
    
    [Serializable]
    private class ItemDTO
    {
        public string id;
        public string displayName;
        public string category;
        public string rarity;
        public string description;
    }
}

