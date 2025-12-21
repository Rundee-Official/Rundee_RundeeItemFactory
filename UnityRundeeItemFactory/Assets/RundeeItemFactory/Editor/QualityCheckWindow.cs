using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Quality check and validation window for generated items
/// </summary>
public class QualityCheckWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string jsonFilePath = "";
    private Vector2 scrollPosition;
    private List<QualityIssue> issues = new List<QualityIssue>();
    private int minQualityScore = 80;
    private bool autoRegenerateLowQuality = false;
    
    private class QualityIssue
    {
        public string itemId;
        public string itemName;
        public int qualityScore;
        public List<string> warnings = new List<string>();
        public List<string> errors = new List<string>();
        public bool isValid;
    }
    
    [MenuItem("Tools/Rundee/Item Factory/Analysis/Quality Check", false, 4010)]
    public static void ShowWindow()
    {
        var window = GetWindow<QualityCheckWindow>("Quality Check");
        window.minSize = new Vector2(700, 500);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Quality Check", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Validate item quality and automatically regenerate low-quality items.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Settings
        EditorGUILayout.LabelField("Quality Settings", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        minQualityScore = EditorGUILayout.IntSlider("Minimum Quality Score:", minQualityScore, 0, 100);
        autoRegenerateLowQuality = EditorGUILayout.Toggle("Auto-regenerate low quality items:", autoRegenerateLowQuality);
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // File Selection
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("JSON File:", GUILayout.Width(80));
        EditorGUILayout.TextField(jsonFilePath, GUILayout.ExpandWidth(true));
        if (GUILayout.Button("Browse", GUILayout.Width(80)))
        {
            string path = EditorUtility.OpenFilePanel("Select JSON File", "", "json");
            if (!string.IsNullOrEmpty(path))
            {
                jsonFilePath = path;
                RunQualityCheck();
            }
        }
        if (GUILayout.Button("Check Quality", GUILayout.Width(120)))
        {
            RunQualityCheck();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Results
        if (issues.Count > 0)
        {
            DrawQualityResults();
        }
        else if (!string.IsNullOrEmpty(jsonFilePath))
        {
            EditorGUILayout.HelpBox("No quality issues found. Click 'Check Quality' to analyze items.", MessageType.Info);
        }
    }
    
    private void DrawQualityResults()
    {
        EditorGUILayout.LabelField("Quality Check Results", EditorStyles.boldLabel);
        
        int validCount = issues.Count(i => i.isValid);
        int invalidCount = issues.Count - validCount;
        int lowQualityCount = issues.Count(i => i.isValid && i.qualityScore < minQualityScore);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.LabelField($"Total Items: {issues.Count}");
        EditorGUILayout.LabelField($"Valid: {validCount} | Invalid: {invalidCount}");
        EditorGUILayout.LabelField($"Low Quality (< {minQualityScore}): {lowQualityCount}");
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        if (lowQualityCount > 0 && autoRegenerateLowQuality)
        {
            EditorGUILayout.HelpBox(
                $"{lowQualityCount} item(s) have quality scores below {minQualityScore}. " +
                "Consider regenerating these items.",
                MessageType.Warning);
            
            if (GUILayout.Button($"Regenerate {lowQualityCount} Low Quality Items", GUILayout.Height(30)))
            {
                RegenerateLowQualityItems();
            }
        }
        
        EditorGUILayout.Space();
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        foreach (var issue in issues.OrderByDescending(i => i.isValid ? i.qualityScore : -1))
        {
            DrawQualityIssue(issue);
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawQualityIssue(QualityIssue issue)
    {
        MessageType messageType = issue.isValid ? MessageType.Info : MessageType.Error;
        if (issue.isValid && issue.qualityScore < minQualityScore)
        {
            messageType = MessageType.Warning;
        }
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField(issue.itemName, EditorStyles.boldLabel);
        EditorGUILayout.LabelField($"Score: {issue.qualityScore}/100", GUILayout.Width(120));
        
        if (!issue.isValid)
        {
            EditorGUILayout.LabelField("INVALID", EditorStyles.boldLabel);
        }
        else if (issue.qualityScore < minQualityScore)
        {
            EditorGUILayout.LabelField("LOW QUALITY", EditorStyles.boldLabel);
        }
        else
        {
            EditorGUILayout.LabelField("OK", EditorStyles.boldLabel);
        }
        
        EditorGUILayout.EndHorizontal();
        
        if (issue.errors.Count > 0)
        {
            EditorGUILayout.LabelField("Errors:", EditorStyles.miniLabel);
            foreach (var error in issue.errors)
            {
                EditorGUILayout.HelpBox(error, MessageType.Error);
            }
        }
        
        if (issue.warnings.Count > 0)
        {
            EditorGUILayout.LabelField("Warnings:", EditorStyles.miniLabel);
            foreach (var warning in issue.warnings)
            {
                EditorGUILayout.HelpBox(warning, MessageType.Warning);
            }
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void RunQualityCheck()
    {
        if (string.IsNullOrEmpty(jsonFilePath) || !File.Exists(jsonFilePath))
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.FileNotFound, jsonFilePath);
            return;
        }
        
        issues.Clear();
        
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
            
            // Simulate quality checking
            // In a real implementation, this would call the actual QualityChecker
            foreach (var item in jsonData.items)
            {
                var issue = new QualityIssue
                {
                    itemId = item.id,
                    itemName = item.displayName,
                    qualityScore = UnityEngine.Random.Range(70, 100), // Simulated
                    isValid = true
                };
                
                // Simulate some warnings
                if (string.IsNullOrEmpty(item.description))
                {
                    issue.warnings.Add("Missing description");
                    issue.qualityScore -= 10;
                }
                
                if (string.IsNullOrEmpty(item.id))
                {
                    issue.errors.Add("Missing ID");
                    issue.isValid = false;
                }
                
                issues.Add(issue);
            }
            
            Debug.Log($"[QualityCheck] Checked {issues.Count} items");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.InvalidJSON, ex.Message);
        }
    }
    
    private void RegenerateLowQualityItems()
    {
        var lowQualityItems = issues.Where(i => i.isValid && i.qualityScore < minQualityScore).ToList();
        
        if (lowQualityItems.Count == 0)
        {
            return;
        }
        
        if (!ErrorHandler.ShowConfirmation("Regenerate Items", 
            $"Regenerate {lowQualityItems.Count} low quality item(s)?"))
        {
            return;
        }
        
        // Implement actual regeneration
        string outputPath = jsonFilePath;
        if (string.IsNullOrEmpty(outputPath))
        {
            ErrorHandler.ShowError("Regeneration", "No JSON file selected. Please select a JSON file first.");
            return;
        }

        // Determine item type from selected items or file name
        ItemType itemType = selectedType;
        
        // If no type selected, try to infer from file name
        if (itemType == ItemType.Food) // Use Food as default, but check filename
        {
            string fileName = Path.GetFileNameWithoutExtension(outputPath).ToLower();
            if (fileName.Contains("food")) itemType = ItemType.Food;
            else if (fileName.Contains("drink")) itemType = ItemType.Drink;
            else if (fileName.Contains("medicine")) itemType = ItemType.Medicine;
            else if (fileName.Contains("material")) itemType = ItemType.Material;
            else if (fileName.Contains("weapon") && !fileName.Contains("component")) itemType = ItemType.Weapon;
            else if (fileName.Contains("weaponcomponent") || fileName.Contains("weapon_component")) itemType = ItemType.WeaponComponent;
            else if (fileName.Contains("ammo")) itemType = ItemType.Ammo;
            else if (fileName.Contains("armor")) itemType = ItemType.Armor;
            else if (fileName.Contains("clothing")) itemType = ItemType.Clothing;
        }

        // Regenerate items
        bool started = ItemFactoryWindow.GenerateItemsAsync(
            itemType,
            lowQualityItems.Count,
            outputPath,
            "llama3", // Default model, could be made configurable
            null, // Default preset, could be made configurable
            null, // No custom preset
            (success, message) =>
            {
                if (success)
                {
                    ErrorHandler.ShowSuccess(
                        $"Successfully regenerated {lowQualityItems.Count} item(s).");
                    // Refresh quality check
                    RunQualityCheck();
                }
                else
                {
                    ErrorHandler.ShowError("Regeneration", 
                        $"Failed to regenerate items: {message}");
                }
            }
        );

        if (!started)
        {
            ErrorHandler.ShowError("Regeneration", "Failed to start regeneration process.");
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

