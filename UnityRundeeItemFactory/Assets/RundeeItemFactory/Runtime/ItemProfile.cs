using System;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Player Settings structure for item generation
/// </summary>
[Serializable]
public class PlayerSettings
{
    public int maxHunger = 100;
    public int maxThirst = 100;
    public int maxHealth = 100;
    public int maxStamina = 100;
    public int maxWeight = 50000; // grams (50kg default)
    public int maxEnergy = 100;
}

/// <summary>
/// Item Profile structure for Unity (matches C++ structure)
/// </summary>
[Serializable]
public class ItemProfile
{
    public string id;
    public string displayName;
    public string description;
    public string itemTypeName;
    public int version = 1;
    public bool isDefault = false;
    public string customContext = "";  // User-defined world context/background
    public List<ProfileField> fields = new List<ProfileField>();
    public Dictionary<string, string> metadata = new Dictionary<string, string>();
    public PlayerSettings playerSettings = new PlayerSettings();  // Player settings for this profile
}

[Serializable]
public class FieldRelationshipConstraint
{
    public string operator_ = ">=";  // ">=", "<=", ">", "<", "==", "!="
    public string targetField = "";
    public double offset = 0.0;
    public string description = "";
}

[Serializable]
public class ProfileField
{
    public string name;
    public FieldType type;
    public string displayName;
    public string description;
    public string category = "";
    public int displayOrder = 0;
    public string defaultValue = "";
    public bool isRequired = false;
    public double minValue = 0;
    public double maxValue = 0;
    public int minLength = 0;
    public int maxLength = 0;
    public List<string> allowedValues = new List<string>();
    public List<FieldRelationshipConstraint> relationshipConstraints = new List<FieldRelationshipConstraint>();
    public string customConstraint = "";  // Custom constraint expression
}

public enum FieldType
{
    String,
    Integer,
    Float,
    Boolean,
    Array,
    Object
}

/// <summary>
/// Player Stat Field - represents a single stat value
/// </summary>
[Serializable]
public class PlayerStatField
{
    public string name;              // Internal name (e.g., "maxHealth")
    public string displayName;       // Display name (e.g., "Max Health")
    public string description;       // Description/tooltip
    public int value;                // Stat value
    public int displayOrder;         // Display order within section
}

/// <summary>
/// Player Stat Section - groups related stats together
/// </summary>
[Serializable]
public class PlayerStatSection
{
    public string name;                              // Section name (e.g., "Vital Stats", "Physical Stats")
    public List<PlayerStatField> fields = new List<PlayerStatField>();
    public int displayOrder;                         // Section display order
}

/// <summary>
/// Player Profile structure for managing player settings
/// </summary>
[Serializable]
public class PlayerProfile
{
    public string id;
    public string displayName;
    public string description;
    public int version = 1;
    public bool isDefault = false;
    public PlayerSettings playerSettings = new PlayerSettings();  // Legacy: kept for backward compatibility
    public List<PlayerStatSection> statSections = new List<PlayerStatSection>();  // New: dynamic stat sections
}



