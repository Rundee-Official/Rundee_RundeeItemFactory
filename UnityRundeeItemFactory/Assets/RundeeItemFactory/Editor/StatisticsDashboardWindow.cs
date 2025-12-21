using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Statistics dashboard for analyzing generated items
/// </summary>
public class StatisticsDashboardWindow : EditorWindow
{
    private ItemType selectedType = ItemType.Food;
    private string jsonFilePath = "";
    private Vector2 scrollPosition;
    private Dictionary<string, int> rarityDistribution = new Dictionary<string, int>();
    private Dictionary<string, int> categoryDistribution = new Dictionary<string, int>();
    private int totalItems = 0;
    private bool showRarityChart = true;
    private bool showCategoryChart = true;
    private bool showTimeSeries = false;
    
    [MenuItem("Tools/Rundee/Item Factory/Analysis/Statistics Dashboard", false, 4000)]
    public static void ShowWindow()
    {
        var window = GetWindow<StatisticsDashboardWindow>("Statistics Dashboard");
        window.minSize = new Vector2(800, 600);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Statistics Dashboard", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Analyze item generation statistics, balance, and distribution patterns.",
            MessageType.Info);
        
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
                AnalyzeData();
            }
        }
        if (GUILayout.Button("Analyze", GUILayout.Width(80)))
        {
            AnalyzeData();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        if (totalItems == 0)
        {
            EditorGUILayout.HelpBox("No data loaded. Select a JSON file and click Analyze.", MessageType.Info);
            return;
        }
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        // Summary
        EditorGUILayout.LabelField("Summary", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.LabelField($"Total Items: {totalItems}");
        EditorGUILayout.LabelField($"Item Type: {selectedType}");
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Rarity Distribution
        showRarityChart = EditorGUILayout.Foldout(showRarityChart, "Rarity Distribution", true);
        if (showRarityChart)
        {
            DrawRarityChart();
        }
        
        EditorGUILayout.Space();
        
        // Category Distribution
        showCategoryChart = EditorGUILayout.Foldout(showCategoryChart, "Category Distribution", true);
        if (showCategoryChart)
        {
            DrawCategoryChart();
        }
        
        EditorGUILayout.Space();
        
        // Balance Analysis
        EditorGUILayout.LabelField("Balance Analysis", EditorStyles.boldLabel);
        DrawBalanceAnalysis();
        
        EditorGUILayout.Space();
        
        // Export Statistics
        if (GUILayout.Button("Export Statistics Report", GUILayout.Height(30)))
        {
            ExportStatisticsReport();
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawRarityChart()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (rarityDistribution.Count == 0)
        {
            EditorGUILayout.HelpBox("No rarity data available.", MessageType.Info);
            EditorGUILayout.EndVertical();
            return;
        }
        
        int maxCount = rarityDistribution.Values.Max();
        float barWidth = 200f;
        float barHeight = 20f;
        
        foreach (var kvp in rarityDistribution.OrderByDescending(x => x.Value))
        {
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField(kvp.Key, GUILayout.Width(100));
            
            float percentage = (float)kvp.Value / totalItems * 100f;
            float barLength = (float)kvp.Value / maxCount * barWidth;
            
            Rect barRect = GUILayoutUtility.GetRect(barWidth, barHeight);
            Rect filledRect = new Rect(barRect.x, barRect.y, barLength, barHeight);
            
            EditorGUI.DrawRect(filledRect, GetRarityColor(kvp.Key));
            EditorGUI.DrawRect(barRect, Color.gray);
            
            EditorGUILayout.LabelField($"{kvp.Value} ({percentage:F1}%)", GUILayout.Width(100));
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawCategoryChart()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (categoryDistribution.Count == 0)
        {
            EditorGUILayout.HelpBox("No category data available.", MessageType.Info);
            EditorGUILayout.EndVertical();
            return;
        }
        
        foreach (var kvp in categoryDistribution.OrderByDescending(x => x.Value))
        {
            float percentage = (float)kvp.Value / totalItems * 100f;
            EditorGUILayout.BeginHorizontal();
            EditorGUILayout.LabelField(kvp.Key, GUILayout.Width(150));
            EditorGUILayout.LabelField($"{kvp.Value} ({percentage:F1}%)", GUILayout.Width(100));
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawBalanceAnalysis()
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        // Check for balance issues
        List<string> warnings = new List<string>();
        
        // Rarity balance check
        if (rarityDistribution.ContainsKey("Common"))
        {
            float commonPercentage = (float)rarityDistribution["Common"] / totalItems * 100f;
            if (commonPercentage < 40f)
            {
                warnings.Add($"Common items are too rare ({commonPercentage:F1}%). Recommended: 40-60%");
            }
            if (commonPercentage > 70f)
            {
                warnings.Add($"Common items are too common ({commonPercentage:F1}%). Recommended: 40-60%");
            }
        }
        
        if (rarityDistribution.ContainsKey("Rare"))
        {
            float rarePercentage = (float)rarityDistribution["Rare"] / totalItems * 100f;
            if (rarePercentage > 30f)
            {
                warnings.Add($"Rare items are too common ({rarePercentage:F1}%). Recommended: 10-20%");
            }
        }
        
        if (warnings.Count == 0)
        {
            EditorGUILayout.HelpBox("No balance issues detected.", MessageType.Info);
        }
        else
        {
            foreach (var warning in warnings)
            {
                EditorGUILayout.HelpBox(warning, MessageType.Warning);
            }
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private Color GetRarityColor(string rarity)
    {
        switch (rarity.ToLower())
        {
            case "common":
                return Color.white;
            case "uncommon":
                return Color.green;
            case "rare":
                return Color.blue;
            default:
                return Color.gray;
        }
    }
    
    private void AnalyzeData()
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
            
            totalItems = jsonData.items.Length;
            rarityDistribution.Clear();
            categoryDistribution.Clear();
            
            foreach (var item in jsonData.items)
            {
                // Count rarity
                if (!string.IsNullOrEmpty(item.rarity))
                {
                    if (!rarityDistribution.ContainsKey(item.rarity))
                        rarityDistribution[item.rarity] = 0;
                    rarityDistribution[item.rarity]++;
                }
                
                // Count category
                if (!string.IsNullOrEmpty(item.category))
                {
                    if (!categoryDistribution.ContainsKey(item.category))
                        categoryDistribution[item.category] = 0;
                    categoryDistribution[item.category]++;
                }
            }
            
            Debug.Log($"[StatisticsDashboard] Analyzed {totalItems} items");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.InvalidJSON, ex.Message);
        }
    }
    
    private void ExportStatisticsReport()
    {
        string path = EditorUtility.SaveFilePanel(
            "Export Statistics Report",
            "",
            $"statistics_report_{DateTime.Now:yyyyMMdd_HHmmss}.txt",
            "txt");
        
        if (string.IsNullOrEmpty(path))
            return;
        
        try
        {
            using (var writer = new StreamWriter(path))
            {
                writer.WriteLine("=== Item Generation Statistics Report ===");
                writer.WriteLine($"Generated: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
                writer.WriteLine($"Item Type: {selectedType}");
                writer.WriteLine($"Total Items: {totalItems}");
                writer.WriteLine();
                
                writer.WriteLine("=== Rarity Distribution ===");
                foreach (var kvp in rarityDistribution.OrderByDescending(x => x.Value))
                {
                    float percentage = (float)kvp.Value / totalItems * 100f;
                    writer.WriteLine($"{kvp.Key}: {kvp.Value} ({percentage:F1}%)");
                }
                writer.WriteLine();
                
                writer.WriteLine("=== Category Distribution ===");
                foreach (var kvp in categoryDistribution.OrderByDescending(x => x.Value))
                {
                    float percentage = (float)kvp.Value / totalItems * 100f;
                    writer.WriteLine($"{kvp.Key}: {kvp.Value} ({percentage:F1}%)");
                }
            }
            
            ErrorHandler.ShowSuccess($"Statistics report exported to: {path}");
        }
        catch (Exception ex)
        {
            ErrorHandler.ShowError(ErrorHandler.ErrorType.PermissionDenied, ex.Message);
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

