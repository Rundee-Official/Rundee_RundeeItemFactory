using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Item Profile Manager Window - Create and manage custom item profiles
/// </summary>
public class ItemProfileManagerWindow : EditorWindow
{
    private Vector2 scrollPosition;
    private Vector2 fieldsScrollPosition;
    private List<ItemProfile> profiles = new List<ItemProfile>();
    private ItemProfile selectedProfile = null;
    private bool showProfileEditor = false;
    private string searchFilter = "";
    private string selectedItemType = "All";
    private string[] availableItemTypes = new string[] { "All" };
    
    // ItemProfile, ProfileField, FieldRelationshipConstraint, FieldType are now defined in Runtime/ItemProfile.cs
    
    private string profilesPath;
    
    // Item Profile Manager is now integrated into ItemFactoryMainWindow as a tab
    // This menu item is removed - use Item Factory window instead
    // [MenuItem("Tools/Rundee/Item Factory/Tools/Item Profile Manager", false, 3021)]
    // public static void ShowWindow()
    // {
    //     var window = GetWindow<ItemProfileManagerWindow>("Item Profile Manager");
    //     window.minSize = new Vector2(900, 600);
    //     window.Show();
    // }
    
    private void OnEnable()
    {
        // Determine profiles directory (project root/profiles)
        profilesPath = Path.Combine(Application.dataPath, "RundeeItemFactory", "ItemProfiles");
        if (!Directory.Exists(profilesPath))
        {
            Directory.CreateDirectory(profilesPath);
        }
        
        LoadProfiles();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Item Profile Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Create and manage custom item profiles. Profiles define the structure of items " +
            "that will be generated. You can add custom fields with types and validation rules.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Toolbar
        EditorGUILayout.BeginHorizontal();
        
        // Item Type Filter (dynamically generated from loaded profiles)
        EditorGUILayout.LabelField("Item Type:", GUILayout.Width(70));
        int currentIndex = Array.IndexOf(availableItemTypes, selectedItemType);
        if (currentIndex < 0) currentIndex = 0;
        int newIndex = EditorGUILayout.Popup(currentIndex, availableItemTypes, GUILayout.Width(150));
        if (newIndex >= 0 && newIndex < availableItemTypes.Length)
        {
            selectedItemType = availableItemTypes[newIndex];
        }
        
        GUILayout.FlexibleSpace();
        
        // Search
        EditorGUILayout.LabelField("Search:", GUILayout.Width(50));
        searchFilter = EditorGUILayout.TextField(searchFilter, GUILayout.Width(200));
        
        // Refresh
        if (GUILayout.Button("Refresh", GUILayout.Width(80)))
        {
            LoadProfiles();
        }
        
        // Create New Profile
        if (GUILayout.Button("New Profile", GUILayout.Width(100)))
        {
            CreateNewProfile();
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // Split view: Profile list and editor
        EditorGUILayout.BeginHorizontal();
        
        // Left: Profile List
        EditorGUILayout.BeginVertical(GUILayout.Width(300));
        DrawProfileList();
        EditorGUILayout.EndVertical();
        
        // Right: Profile Editor
        EditorGUILayout.BeginVertical();
        if (selectedProfile != null)
        {
            DrawProfileEditor();
        }
        else
        {
            EditorGUILayout.HelpBox("Select a profile to edit, or create a new one.", MessageType.Info);
        }
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.EndHorizontal();
    }
    
    private void DrawProfileList()
    {
        string filterLabel = selectedItemType == "All" ? "All Profiles" : $"Profiles ({selectedItemType})";
        EditorGUILayout.LabelField(filterLabel, EditorStyles.boldLabel);
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition, EditorStyles.helpBox);
        
        var filteredProfiles = profiles.Where(p => 
            (selectedItemType == "All" || p.itemTypeName == selectedItemType) &&
            (string.IsNullOrEmpty(searchFilter) || 
             p.displayName.IndexOf(searchFilter, StringComparison.OrdinalIgnoreCase) >= 0 ||
             p.id.IndexOf(searchFilter, StringComparison.OrdinalIgnoreCase) >= 0))
            .OrderBy(p => p.isDefault ? 0 : 1)
            .ThenBy(p => p.itemTypeName)
            .ThenBy(p => p.displayName)
            .ToList();
        
        if (filteredProfiles.Count == 0)
        {
            EditorGUILayout.HelpBox("No profiles found. Create a new profile to get started.", MessageType.Info);
        }
        else
        {
            foreach (var profile in filteredProfiles)
            {
                DrawProfileListItem(profile);
            }
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawProfileListItem(ItemProfile profile)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        bool isSelected = selectedProfile == profile;
        bool newSelected = EditorGUILayout.Toggle(isSelected, GUILayout.Width(20));
        
        if (newSelected != isSelected)
        {
            selectedProfile = newSelected ? profile : null;
            if (selectedProfile != null)
            {
                EnsureIdentityFields(selectedProfile);
            }
            showProfileEditor = newSelected;
            GUI.FocusControl(null);
        }
        
        EditorGUILayout.BeginVertical();
        EditorGUILayout.LabelField(profile.displayName, EditorStyles.boldLabel);
        if (!string.IsNullOrEmpty(profile.description))
        {
            EditorGUILayout.LabelField(profile.description, EditorStyles.miniLabel);
        }
        EditorGUILayout.LabelField("ID: " + profile.id + (profile.isDefault ? " (Default)" : ""), EditorStyles.miniLabel);
        int customFieldsCount = profile.fields != null ? profile.fields.Count(f => !IsIdentityField(f.name)) : 0;
        EditorGUILayout.LabelField("Custom Fields: " + customFieldsCount, EditorStyles.miniLabel);
        EditorGUILayout.EndVertical();
        
        GUILayout.FlexibleSpace();
        
        if (GUILayout.Button("Delete", GUILayout.Width(60)))
        {
            if (EditorUtility.DisplayDialog("Delete Profile", 
                "Are you sure you want to delete profile '" + profile.displayName + "'?", 
                "Delete", "Cancel"))
            {
                DeleteProfile(profile);
            }
        }
        
        EditorGUILayout.EndHorizontal();
        EditorGUILayout.EndVertical();
    }
    
    private void DrawProfileEditor()
    {
        if (selectedProfile == null) return;
        
        EditorGUILayout.LabelField("Edit Profile", EditorStyles.boldLabel);
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        
        // Basic Info
        EditorGUILayout.LabelField("Basic Information", EditorStyles.boldLabel);
        
        EditorGUILayout.LabelField("Display Name:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("User-friendly name shown in the profile list (e.g., 'Food Items - Forest Setting')", MessageType.None);
        selectedProfile.displayName = EditorGUILayout.TextField(selectedProfile.displayName);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("ID:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Unique identifier for this profile (e.g., 'food_forest_profile'). Used internally and in file names.", MessageType.None);
        selectedProfile.id = EditorGUILayout.TextField(selectedProfile.id);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("Item Type:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Type of items this profile generates. You can use any custom type name (e.g., 'Food', 'Weapon', 'CustomItemType')", MessageType.None);
        selectedProfile.itemTypeName = EditorGUILayout.TextField(selectedProfile.itemTypeName);
        
        EditorGUILayout.Space(5);
        
        EditorGUILayout.LabelField("Description:", EditorStyles.miniLabel);
        EditorGUILayout.HelpBox("Optional description of this profile's purpose and usage", MessageType.None);
        selectedProfile.description = EditorGUILayout.TextArea(selectedProfile.description, GUILayout.Height(60));
        
        EditorGUILayout.Space(5);
        
        selectedProfile.isDefault = EditorGUILayout.Toggle("Is Default Profile", selectedProfile.isDefault);
        
        EditorGUILayout.Space();
        
        // Custom Context (World Background)
        EditorGUILayout.LabelField("World Context / Background", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Describe the world setting and background. This text will be used in prompts to guide item generation. " +
            "You can describe the environment, theme, style, etc. Leave empty to use preset context.",
            MessageType.Info);
        selectedProfile.customContext = EditorGUILayout.TextArea(selectedProfile.customContext, GUILayout.Height(100));
        
        EditorGUILayout.Space();
        
        // Player Settings
        EditorGUILayout.LabelField("Player Settings", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Player statistics used for item generation. These values define the maximum values for player attributes " +
            "and are used to balance generated items appropriately.",
            MessageType.Info);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Hunger:", GUILayout.Width(120));
        selectedProfile.playerSettings.maxHunger = EditorGUILayout.IntField(selectedProfile.playerSettings.maxHunger);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Thirst:", GUILayout.Width(120));
        selectedProfile.playerSettings.maxThirst = EditorGUILayout.IntField(selectedProfile.playerSettings.maxThirst);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Health:", GUILayout.Width(120));
        selectedProfile.playerSettings.maxHealth = EditorGUILayout.IntField(selectedProfile.playerSettings.maxHealth);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Stamina:", GUILayout.Width(120));
        selectedProfile.playerSettings.maxStamina = EditorGUILayout.IntField(selectedProfile.playerSettings.maxStamina);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Weight (grams):", GUILayout.Width(120));
        selectedProfile.playerSettings.maxWeight = EditorGUILayout.IntField(selectedProfile.playerSettings.maxWeight);
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField("Max Energy:", GUILayout.Width(120));
        selectedProfile.playerSettings.maxEnergy = EditorGUILayout.IntField(selectedProfile.playerSettings.maxEnergy);
        EditorGUILayout.EndHorizontal();
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Identity Fields (Fixed)
        EditorGUILayout.LabelField("Identity Fields (Required)", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "These fields are always included in every item. They define the basic identity and classification of items.",
            MessageType.Info);
        
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        DrawIdentityField("id", "ID", "Unique item identifier (e.g., 'food_canned_beans')", true);
        DrawIdentityField("displayName", "Display Name", "User-facing name (e.g., 'Canned Beans')", true);
        DrawIdentityField("category", "Category", "Item category (e.g., 'Food', 'Weapon')", false);
        DrawIdentityField("rarity", "Rarity", "Item rarity: Common, Uncommon, or Rare", false);
        DrawIdentityField("description", "Description", "Item description text", false);
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Custom Fields
        EditorGUILayout.LabelField("Custom Fields (" + GetCustomFieldsCount() + ")", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Add custom fields specific to your item type. These fields will be included in generated items.",
            MessageType.Info);
        
        fieldsScrollPosition = EditorGUILayout.BeginScrollView(fieldsScrollPosition, GUILayout.Height(300));
        
        for (int i = 0; i < selectedProfile.fields.Count; i++)
        {
            // Skip identity fields
            if (IsIdentityField(selectedProfile.fields[i].name))
                continue;
                
            DrawFieldEditor(selectedProfile.fields[i], i);
        }
        
        EditorGUILayout.EndScrollView();
        
        // Add Field Button
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Add Custom Field", GUILayout.Height(30)))
        {
            var newField = new ProfileField
            {
                name = "newField",
                type = FieldType.String,
                displayName = "New Field",
                category = "General",
                displayOrder = GetCustomFieldsCount()
            };
            selectedProfile.fields.Add(newField);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndVertical();
        
        EditorGUILayout.Space();
        
        // Save Button
        EditorGUILayout.BeginHorizontal();
        GUILayout.FlexibleSpace();
        if (GUILayout.Button("Save Profile", GUILayout.Width(120), GUILayout.Height(30)))
        {
            SaveProfile(selectedProfile);
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.EndScrollView();
    }
    
    // Common field categories
    private static readonly string[] FieldCategories = new string[]
    {
        "Identity",        // id, displayName
        "Classification",  // category, rarity
        "Stats",           // hungerRestore, healthRestore, damage
        "Properties",      // spoils, materialType, weaponType
        "Combat",          // accuracy, recoil, damage
        "Inventory",       // maxStack, weight
        "Effects",         // restore values, modifiers
        "General"          // 기타
    };
    
    // Identity field names (always included)
    private static readonly string[] IdentityFieldNames = new string[]
    {
        "id",
        "displayName",
        "category",
        "rarity",
        "description"
    };
    
    private bool IsIdentityField(string fieldName)
    {
        return Array.IndexOf(IdentityFieldNames, fieldName) >= 0;
    }
    
    private int GetCustomFieldsCount()
    {
        if (selectedProfile == null) return 0;
        return selectedProfile.fields.Count(f => !IsIdentityField(f.name));
    }
    
    private void EnsureIdentityFields(ItemProfile profile)
    {
        if (profile == null || profile.fields == null) return;
        
        // Check and add missing identity fields
        foreach (string fieldName in IdentityFieldNames)
        {
            if (!profile.fields.Any(f => f.name == fieldName))
            {
                var field = CreateIdentityField(fieldName);
                profile.fields.Add(field);
            }
        }
        
        // Sort fields: identity fields first, then custom fields
        profile.fields = profile.fields.OrderBy(f => 
        {
            int identityIndex = Array.IndexOf(IdentityFieldNames, f.name);
            return identityIndex >= 0 ? identityIndex : 1000 + f.displayOrder;
        }).ToList();
    }
    
    private ProfileField CreateIdentityField(string fieldName)
    {
        string displayName = "";
        string description = "";
        bool isRequired = false;
        
        switch (fieldName)
        {
            case "id":
                displayName = "ID";
                description = "Unique item identifier (e.g., 'food_canned_beans')";
                isRequired = true;
                break;
            case "displayName":
                displayName = "Display Name";
                description = "User-facing name (e.g., 'Canned Beans')";
                isRequired = true;
                break;
            case "category":
                displayName = "Category";
                description = "Item category (e.g., 'Food', 'Weapon')";
                isRequired = false;
                break;
            case "rarity":
                displayName = "Rarity";
                description = "Item rarity: Common, Uncommon, or Rare";
                isRequired = false;
                break;
            case "description":
                displayName = "Description";
                description = "Item description text";
                isRequired = false;
                break;
        }
        
        var field = new ProfileField
        {
            name = fieldName,
            displayName = displayName,
            description = description,
            type = FieldType.String,
            category = "Identity",
            isRequired = isRequired,
            displayOrder = Array.IndexOf(IdentityFieldNames, fieldName)
        };
        
        // Set default validation for specific fields
        if (fieldName == "id")
        {
            field.minLength = 1;
            field.maxLength = 100;
        }
        else if (fieldName == "displayName")
        {
            field.minLength = 1;
            field.maxLength = 100;
        }
        else if (fieldName == "rarity")
        {
            field.allowedValues = new List<string> { "Common", "Uncommon", "Rare" };
        }
        else if (fieldName == "description")
        {
            field.minLength = 0;
            field.maxLength = 500;
        }
        
        return field;
    }
    
    private void DrawIdentityField(string fieldName, string displayName, string description, bool isRequired)
    {
        // Find or create identity field
        ProfileField field = selectedProfile.fields.FirstOrDefault(f => f.name == fieldName);
        if (field == null)
        {
            field = CreateIdentityField(fieldName);
            selectedProfile.fields.Add(field);
        }
        
        // Display as read-only info (identity fields are fixed)
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField(displayName + ":", GUILayout.Width(120));
        EditorGUILayout.LabelField(field.type.ToString() + (isRequired ? " (Required)" : " (Optional)"), 
            EditorStyles.miniLabel);
        GUILayout.FlexibleSpace();
        EditorGUILayout.EndHorizontal();
        
        if (!string.IsNullOrEmpty(description))
        {
            EditorGUILayout.HelpBox(description, MessageType.None);
        }
    }
    
    private void DrawFieldEditor(ProfileField field, int index)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        EditorGUILayout.LabelField("Field " + (index + 1), EditorStyles.boldLabel, GUILayout.Width(80));
        
        GUILayout.FlexibleSpace();
        
        // Prevent removal of identity fields
        bool isIdentity = IsIdentityField(field.name);
        EditorGUI.BeginDisabledGroup(isIdentity);
        if (GUILayout.Button("Remove", GUILayout.Width(60)))
        {
            selectedProfile.fields.RemoveAt(index);
            return;
        }
        EditorGUI.EndDisabledGroup();
        
        if (isIdentity)
        {
            EditorGUILayout.HelpBox("Identity fields cannot be removed.", MessageType.Info);
        }
        
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space(5);
        
        // Basic Info
        field.name = EditorGUILayout.TextField("Name:", field.name);
        field.displayName = EditorGUILayout.TextField("Display Name:", field.displayName);
        field.description = EditorGUILayout.TextArea(field.description, GUILayout.Height(40));
        
        EditorGUILayout.Space(5);
        
        // Category (Dropdown)
        EditorGUILayout.LabelField("Category:", EditorStyles.miniLabel);
        int categoryIndex = Array.IndexOf(FieldCategories, field.category);
        if (categoryIndex < 0) categoryIndex = FieldCategories.Length - 1; // Default to "General"
        int newCategoryIndex = EditorGUILayout.Popup(categoryIndex, FieldCategories);
        field.category = FieldCategories[newCategoryIndex];
        
        EditorGUILayout.Space(5);
        
        // Type and Settings
        field.type = (FieldType)EditorGUILayout.EnumPopup("Type:", field.type);
        field.displayOrder = EditorGUILayout.IntField("Display Order:", field.displayOrder);
        field.isRequired = EditorGUILayout.Toggle("Required:", field.isRequired);
        field.defaultValue = EditorGUILayout.TextField("Default Value:", field.defaultValue);
        
        EditorGUILayout.Space(5);
        
        // Type-specific Validation
        DrawTypeSpecificValidation(field);
        
        EditorGUILayout.EndVertical();
        EditorGUILayout.Space();
    }
    
    private void DrawTypeSpecificValidation(ProfileField field)
    {
        EditorGUILayout.LabelField("Validation", EditorStyles.boldLabel);
        
        switch (field.type)
        {
            case FieldType.String:
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Min Length:", GUILayout.Width(80));
                field.minLength = EditorGUILayout.IntField(field.minLength, GUILayout.Width(100));
                EditorGUILayout.LabelField("Max Length:", GUILayout.Width(80));
                field.maxLength = EditorGUILayout.IntField(field.maxLength, GUILayout.Width(100));
                EditorGUILayout.EndHorizontal();
                
                // Allowed Values (for String with enum-like behavior)
                EditorGUILayout.Space(3);
                EditorGUILayout.LabelField("Allowed Values (comma-separated, optional):", EditorStyles.miniLabel);
                string allowedValuesStr = string.Join(", ", field.allowedValues);
                allowedValuesStr = EditorGUILayout.TextField(allowedValuesStr);
                if (allowedValuesStr != string.Join(", ", field.allowedValues))
                {
                    field.allowedValues = allowedValuesStr.Split(',')
                        .Select(v => v.Trim())
                        .Where(v => !string.IsNullOrEmpty(v))
                        .ToList();
                }
                break;
                
            case FieldType.Integer:
            case FieldType.Float:
                EditorGUILayout.BeginHorizontal();
                EditorGUILayout.LabelField("Min Value:", GUILayout.Width(80));
                field.minValue = EditorGUILayout.DoubleField(field.minValue, GUILayout.Width(100));
                EditorGUILayout.LabelField("Max Value:", GUILayout.Width(80));
                field.maxValue = EditorGUILayout.DoubleField(field.maxValue, GUILayout.Width(100));
                EditorGUILayout.EndHorizontal();
                break;
                
            case FieldType.Boolean:
                EditorGUILayout.HelpBox("Boolean fields don't require validation.", MessageType.None);
                break;
                
            case FieldType.Array:
            case FieldType.Object:
                EditorGUILayout.HelpBox("Complex types (Array/Object) validation is handled by the parser.", MessageType.Info);
                break;
        }
    }
    
    private void LoadProfiles()
    {
        profiles.Clear();
        
        if (!Directory.Exists(profilesPath))
        {
            UpdateAvailableItemTypes();
            return;
        }
        
        string[] files = Directory.GetFiles(profilesPath, "*.json");
        foreach (string file in files)
        {
            try
            {
                string json = File.ReadAllText(file);
                ItemProfile profile = JsonUtility.FromJson<ItemProfile>(json);
                
                // Ensure identity fields exist
                EnsureIdentityFields(profile);
                if (profile != null)
                {
                    profiles.Add(profile);
                }
            }
            catch (Exception e)
            {
                UnityEngine.Debug.LogError($"Failed to load profile from {file}: {e.Message}");
            }
        }
        
        UpdateAvailableItemTypes();
    }
    
    private void UpdateAvailableItemTypes()
    {
        // Collect unique item types from loaded profiles
        var itemTypes = profiles
            .Where(p => !string.IsNullOrEmpty(p.itemTypeName))
            .Select(p => p.itemTypeName)
            .Distinct()
            .OrderBy(t => t)
            .ToList();
        
        // Create array with "All" option first, then unique item types
        availableItemTypes = new string[] { "All" }.Concat(itemTypes).ToArray();
        
        // If current selection is not in available types, reset to "All"
        if (!availableItemTypes.Contains(selectedItemType))
        {
            selectedItemType = "All";
        }
    }
    
    private void SaveProfile(ItemProfile profile)
    {
        if (string.IsNullOrEmpty(profile.id))
        {
            EditorUtility.DisplayDialog("Error", "Profile ID cannot be empty.", "OK");
            return;
        }
        
        string filePath = Path.Combine(profilesPath, profile.id + ".json");
        try
        {
            string json = JsonUtility.ToJson(profile, true);
            File.WriteAllText(filePath, json);
            AssetDatabase.Refresh();
            
            LoadProfiles();
            EditorUtility.DisplayDialog("Success", "Profile saved successfully.", "OK");
        }
        catch (Exception e)
        {
            EditorUtility.DisplayDialog("Error", "Failed to save profile: " + e.Message, "OK");
        }
    }
    
    private void DeleteProfile(ItemProfile profile)
    {
        string filePath = Path.Combine(profilesPath, profile.id + ".json");
        if (File.Exists(filePath))
        {
            File.Delete(filePath);
            profiles.Remove(profile);
            if (selectedProfile == profile)
            {
                selectedProfile = null;
                showProfileEditor = false;
            }
            AssetDatabase.Refresh();
        }
    }
    
    private void CreateNewProfile()
    {
        var newProfile = new ItemProfile
        {
            id = "profile_" + System.Guid.NewGuid().ToString().Substring(0, 8),
            displayName = "New Profile",
            description = "",
            itemTypeName = "", // User will set this manually
            version = 1,
            isDefault = false,
            fields = new List<ProfileField>()
        };
        
        // Ensure identity fields are added
        EnsureIdentityFields(newProfile);
        
        profiles.Add(newProfile);
        selectedProfile = newProfile;
        showProfileEditor = true;
    }
}

