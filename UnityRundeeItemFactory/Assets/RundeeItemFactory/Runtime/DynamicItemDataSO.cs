using System;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Dynamic ScriptableObject for profile-based items
/// </summary>
/// <remarks>
/// This ScriptableObject stores items with dynamic fields based on profiles.
/// Uses a Dictionary to store field values, allowing flexibility for custom item structures.
/// </remarks>
[CreateAssetMenu(
    fileName = "DynamicItemData",
    menuName = "Rundee/Items/Dynamic Item Data",
    order = 999)]
public class DynamicItemDataSO : ScriptableObject
{
    [Header("Item Profile Info")]
    /// <summary>Profile ID used to generate this item</summary>
    public string profileId;
    
    /// <summary>Item type name (e.g., "Food", "Weapon")</summary>
    public string itemTypeName;
    
    [Header("Dynamic Fields")]
    /// <summary>Dynamic field values (field name -> value as string)</summary>
    /// <remarks>
    /// Values are stored as strings and can be parsed based on field type.
    /// For numeric types, use int.Parse() or float.Parse().
    /// For boolean, use bool.Parse().
    /// Note: Dictionary cannot be serialized by Unity, so we use a workaround with
    /// serializable key-value pairs.
    /// </remarks>
    [SerializeField]
    private List<SerializableKeyValuePair> serializedFields = new List<SerializableKeyValuePair>();
    
    private Dictionary<string, string> _fieldValues;
    
    /// <summary>Field values dictionary (populated from serializedFields)</summary>
    public Dictionary<string, string> fieldValues
    {
        get
        {
            if (_fieldValues == null || _fieldValues.Count == 0)
            {
                _fieldValues = new Dictionary<string, string>();
                foreach (var kvp in serializedFields)
                {
                    if (!string.IsNullOrEmpty(kvp.key))
                    {
                        _fieldValues[kvp.key] = kvp.value ?? "";
                    }
                }
            }
            return _fieldValues;
        }
        set
        {
            _fieldValues = value;
            serializedFields.Clear();
            if (value != null)
            {
                foreach (var kvp in value)
                {
                    serializedFields.Add(new SerializableKeyValuePair { key = kvp.Key, value = kvp.Value ?? "" });
                }
            }
        }
    }
    
    [Serializable]
    private class SerializableKeyValuePair
    {
        public string key;
        public string value;
    }
    
    /// <summary>Get field value as string</summary>
    public string GetFieldValue(string fieldName)
    {
        return fieldValues.TryGetValue(fieldName, out string value) ? value : null;
    }
    
    /// <summary>Get field value as integer</summary>
    public int GetFieldInt(string fieldName, int defaultValue = 0)
    {
        if (fieldValues.TryGetValue(fieldName, out string value) && int.TryParse(value, out int result))
        {
            return result;
        }
        return defaultValue;
    }
    
    /// <summary>Get field value as float</summary>
    public float GetFieldFloat(string fieldName, float defaultValue = 0f)
    {
        if (fieldValues.TryGetValue(fieldName, out string value) && float.TryParse(value, out float result))
        {
            return result;
        }
        return defaultValue;
    }
    
    /// <summary>Get field value as boolean</summary>
    public bool GetFieldBool(string fieldName, bool defaultValue = false)
    {
        if (fieldValues.TryGetValue(fieldName, out string value) && bool.TryParse(value, out bool result))
        {
            return result;
        }
        return defaultValue;
    }
    
    /// <summary>Set field value</summary>
    public void SetFieldValue(string fieldName, object value)
    {
        fieldValues[fieldName] = value?.ToString() ?? "";
        // Update serialized fields
        var existing = serializedFields.Find(x => x.key == fieldName);
        if (existing != null)
        {
            existing.value = fieldValues[fieldName];
        }
        else
        {
            serializedFields.Add(new SerializableKeyValuePair { key = fieldName, value = fieldValues[fieldName] });
        }
    }
    
    /// <summary>Get ID field (common field)</summary>
    public string GetId()
    {
        return GetFieldValue("id") ?? "";
    }
    
    /// <summary>Get display name field (common field)</summary>
    public string GetDisplayName()
    {
        return GetFieldValue("displayName") ?? "";
    }
    
    /// <summary>Get category field (common field)</summary>
    public string GetCategory()
    {
        return GetFieldValue("category") ?? "";
    }
    
    /// <summary>Get rarity field (common field)</summary>
    public string GetRarity()
    {
        return GetFieldValue("rarity") ?? "";
    }
}
