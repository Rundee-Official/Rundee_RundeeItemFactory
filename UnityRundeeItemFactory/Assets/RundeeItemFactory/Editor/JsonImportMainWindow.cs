using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// JSON Import window for importing generated items using profiles
/// </summary>
public class JsonImportMainWindow : EditorWindow
{
    private string profilesPath;
    private List<ItemProfile> availableProfiles = new List<ItemProfile>();
    private ItemProfile selectedProfile = null;
    private int selectedProfileIndex = 0;
    private string[] profileNames = new string[0];
    
    private string jsonFilePath = "";
    private Vector2 scrollPosition;
    
    [MenuItem("Tools/Rundee/Item Factory/Import JSON", false, 2000)]
    public static void ShowWindow()
    {
        var window = GetWindow<JsonImportMainWindow>("Import JSON");
        window.minSize = new Vector2(500, 400);
        window.LoadProfiles();
    }
    
    private void OnEnable()
    {
        LoadProfiles();
    }
    
    private void OnGUI()
    {
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        EditorGUILayout.Space(10);
        EditorGUILayout.LabelField("Import JSON Items", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Import generated JSON items into Unity ScriptableObjects using a profile. " +
            "The profile defines the structure of the items.",
            MessageType.Info);
        
        EditorGUILayout.Space(10);
        
        // Profile Selection
        EditorGUILayout.LabelField("Item Profile", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        if (availableProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No profiles found. Please create profiles using 'Tools/Rundee/Item Factory/Tools/Item Profile Manager'", 
                MessageType.Warning);
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadProfiles();
            }
            if (GUILayout.Button("Open Item Factory"))
            {
                ItemFactoryMainWindow.ShowWindow();
            }
            EditorGUILayout.EndHorizontal();
        }
        else
        {
            EditorGUILayout.LabelField("Select Profile:", EditorStyles.miniLabel);
            int newIndex = EditorGUILayout.Popup(selectedProfileIndex, profileNames);
            if (newIndex != selectedProfileIndex && newIndex >= 0 && newIndex < availableProfiles.Count)
            {
                selectedProfileIndex = newIndex;
                selectedProfile = availableProfiles[newIndex];
            }
            
            if (selectedProfile != null)
            {
                EditorGUILayout.Space(5);
                EditorGUILayout.LabelField("Profile Info:", EditorStyles.miniLabel);
                EditorGUILayout.LabelField($"Name: {selectedProfile.displayName}", EditorStyles.wordWrappedLabel);
                EditorGUILayout.LabelField($"Type: {selectedProfile.itemTypeName}", EditorStyles.wordWrappedLabel);
            }
            
            EditorGUILayout.BeginHorizontal();
            if (GUILayout.Button("Refresh Profiles"))
            {
                LoadProfiles();
            }
            if (GUILayout.Button("Open Item Factory"))
            {
                ItemFactoryMainWindow.ShowWindow();
            }
            EditorGUILayout.EndHorizontal();
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // JSON File Selection
        EditorGUILayout.LabelField("JSON File", EditorStyles.boldLabel);
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        EditorGUILayout.LabelField("JSON File Path:", EditorStyles.miniLabel);
        EditorGUILayout.BeginHorizontal();
        jsonFilePath = EditorGUILayout.TextField(jsonFilePath);
        if (GUILayout.Button("Browse", GUILayout.Width(70)))
        {
            string path = EditorUtility.OpenFilePanel("Select JSON File", 
                Path.GetDirectoryName(jsonFilePath), "json");
            if (!string.IsNullOrEmpty(path))
            {
                jsonFilePath = path;
            }
        }
        EditorGUILayout.EndHorizontal();
        
        if (!string.IsNullOrEmpty(jsonFilePath) && !File.Exists(jsonFilePath))
        {
            EditorGUILayout.HelpBox("File not found", MessageType.Warning);
        }
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space(10);
        
        // Import Button
        EditorGUI.BeginDisabledGroup(selectedProfile == null || !File.Exists(jsonFilePath));
        if (GUILayout.Button("Import Items", GUILayout.Height(40)))
        {
            ImportItems();
        }
        EditorGUI.EndDisabledGroup();
        
        EditorGUILayout.EndScrollView();
    }
    
    private void LoadProfiles()
    {
        availableProfiles.Clear();
        
        // Determine profiles directory (project root/profiles)
        profilesPath = Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles");
        if (!Directory.Exists(profilesPath))
        {
            Directory.CreateDirectory(profilesPath);
            return;
        }
        
        string[] files = Directory.GetFiles(profilesPath, "*.json");
        foreach (string file in files)
        {
            try
            {
                string json = File.ReadAllText(file);
                ItemProfile profile = JsonUtility.FromJson<ItemProfile>(json);
                if (profile != null)
                {
                    availableProfiles.Add(profile);
                }
            }
            catch (Exception e)
            {
                Debug.LogWarning($"[JsonImportMainWindow] Failed to load profile {file}: {e.Message}");
            }
        }
        
        profileNames = availableProfiles.Select(p => $"{p.displayName} ({p.id})").ToArray();
        
        if (availableProfiles.Count > 0 && selectedProfile == null)
        {
            selectedProfileIndex = 0;
            selectedProfile = availableProfiles[0];
        }
    }
    
    private void ImportItems()
    {
        if (selectedProfile == null)
        {
            EditorUtility.DisplayDialog("Error", "Please select a profile", "OK");
            return;
        }
        
        if (!File.Exists(jsonFilePath))
        {
            EditorUtility.DisplayDialog("Error", "JSON file not found", "OK");
            return;
        }
        
        try
        {
            int imported = DynamicItemImporter.ImportFromJson(jsonFilePath, selectedProfile.id, selectedProfile.itemTypeName);
            EditorUtility.DisplayDialog("Success", 
                $"Successfully imported {imported} items from {Path.GetFileName(jsonFilePath)}", "OK");
        }
        catch (Exception e)
        {
            EditorUtility.DisplayDialog("Import Error", 
                $"Failed to import items:\n{e.Message}", "OK");
            Debug.LogError($"[JsonImportMainWindow] Import failed: {e.Message}\n{e.StackTrace}");
        }
    }
}



