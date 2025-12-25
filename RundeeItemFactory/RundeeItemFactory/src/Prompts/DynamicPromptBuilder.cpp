/**
 * @file DynamicPromptBuilder.cpp
 * @brief Implementation of dynamic prompt builder
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Prompts/DynamicPromptBuilder.h"
#include "Prompts/PromptTemplateLoader.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

std::string DynamicPromptBuilder::BuildPromptFromProfile(
    const ItemProfile& profile,
    const PlayerProfile& playerProfile,
    const FoodGenerateParams& params,
    const std::set<std::string>& existingIds,
    const std::string& modelName,
    const std::string& generationTimestamp,
    int existingCount)
{
    std::ostringstream prompt;
    
    // Try to load template first (if templates are used in the future)
    std::string templateName = profile.itemTypeName;
    std::transform(templateName.begin(), templateName.end(), templateName.begin(), ::tolower);
    templateName += "_template";
    
    std::string worldContext = profile.customContext.empty() ? "" : profile.customContext;
    std::string templateContent = PromptTemplateLoader::LoadTemplate(
        templateName,
        worldContext,
        params.maxHunger,
        params.maxThirst,
        params.count,
        existingIds,
        profile.displayName,  // Used as presetName replacement (display name of profile)
        profile.itemTypeName,
        modelName,
        generationTimestamp,
        existingCount);
    
    // If template exists, use it as base
    if (!templateContent.empty())
    {
        prompt << templateContent;
    }
    else
    {
        // Build prompt from scratch
        prompt << "You are a game item generator. Generate " << params.count 
               << " unique " << profile.itemTypeName << " items as a JSON array.\n\n";
        
        // Use Item Profile's customContext as world context (Preset system removed)
        if (!profile.customContext.empty())
        {
            prompt << "World Context:\n" << profile.customContext << "\n\n";
        }
    }
    
    // Add profile information section
    prompt << "\n=== ITEM PROFILE SPECIFICATION ===\n\n";
    prompt << "Profile: " << profile.displayName << " (" << profile.id << ")\n";
    if (!profile.description.empty())
    {
        prompt << "Description: " << profile.description << "\n";
    }
    prompt << "Item Type: " << profile.itemTypeName << "\n";
    
    // Note: customContext is already included in "World Context" section above, so we don't duplicate it here
    
    // Add metadata if provided
    if (!profile.metadata.empty())
    {
        prompt << "Metadata: ";
        bool first = true;
        for (const auto& [key, value] : profile.metadata)
        {
            if (!first) prompt << ", ";
            prompt << key << ": " << value.dump();
            first = false;
        }
        prompt << "\n";
    }
    prompt << "\n";
    
    // Add player profile information
    prompt << "\n=== PLAYER PROFILE SPECIFICATION ===\n\n";
    prompt << "Player Profile: " << playerProfile.displayName << " (" << playerProfile.id << ")\n";
    if (!playerProfile.description.empty())
    {
        prompt << "Description: " << playerProfile.description << "\n";
    }
    prompt << "\n";
    
    // Add player settings for balance context
    prompt << "Player Stat Maximums (for balancing):\n";
    prompt << "  - Max Hunger: " << params.maxHunger << "\n";
    prompt << "  - Max Thirst: " << params.maxThirst << "\n";
    prompt << "  - Max Health: " << params.maxHealth << "\n";
    prompt << "  - Max Stamina: " << params.maxStamina << "\n";
    prompt << "  - Max Weight: " << params.maxWeight << " grams\n";
    prompt << "  - Max Energy: " << params.maxEnergy << "\n";
    prompt << "\n";
    
    // Add player stat sections (custom stat fields)
    if (!playerProfile.statSections.empty())
    {
        prompt << "Player Stat Sections (Additional Context):\n";
        
        // Sort sections by displayOrder
        std::vector<PlayerStatSection> sortedSections = playerProfile.statSections;
        std::sort(sortedSections.begin(), sortedSections.end(),
            [](const PlayerStatSection& a, const PlayerStatSection& b) {
                return a.displayOrder < b.displayOrder;
            });
        
        for (const auto& section : sortedSections)
        {
            prompt << "\n  Section: " << section.displayName;
            if (!section.name.empty() && section.name != section.displayName)
            {
                prompt << " (" << section.name << ")";
            }
            prompt << "\n";
            
            if (!section.description.empty())
            {
                prompt << "    Description: " << section.description << "\n";
            }
            
            if (!section.fields.empty())
            {
                // Sort fields by displayOrder
                std::vector<PlayerStatField> sortedFields = section.fields;
                std::sort(sortedFields.begin(), sortedFields.end(),
                    [](const PlayerStatField& a, const PlayerStatField& b) {
                        return a.displayOrder < b.displayOrder;
                    });
                
                prompt << "    Fields:\n";
                for (const auto& field : sortedFields)
                {
                    prompt << "      - " << field.displayName;
                    if (!field.name.empty() && field.name != field.displayName)
                    {
                        prompt << " (" << field.name << ")";
                    }
                    prompt << ": " << field.value;
                    if (!field.description.empty())
                    {
                        prompt << " - " << field.description;
                    }
                    prompt << "\n";
                }
            }
        }
        prompt << "\n";
    }
    
    // Add all field definitions with validation rules
    prompt << "=== REQUIRED FIELDS AND VALIDATION RULES ===\n\n";
    prompt << "CRITICAL: Every item MUST have 'id' and 'displayName' fields. These are ALWAYS required and must be generated FIRST.\n";
    prompt << "- 'displayName': Human-readable name that clearly identifies the item (e.g., 'AK-47 Assault Rifle', 'FN SCAR-17S Enhanced', 'Healing Potion').\n";
    prompt << "- 'id': Unique identifier based on displayName in format {itemType}_{cleanedDisplayName} (e.g., weapon_ak47assaultrifle, weapon_fnscar17senhanced, food_healingpotion).\n";
    prompt << "  The ID should be derived from the displayName by:\n";
    prompt << "  1. Converting to lowercase\n";
    prompt << "  2. Removing spaces, hyphens, and special characters\n";
    prompt << "  3. Keeping only alphanumeric characters\n";
    prompt << "  4. Prefixing with item type (e.g., 'weapon_', 'food_')\n";
    prompt << "  Example: 'FN SCAR-17S Enhanced' -> 'weapon_fnscar17senhanced'\n\n";
    prompt << "Each item MUST include the following fields with these specifications:\n\n";
    
    // Sort fields by display order
    std::vector<ProfileField> sortedFields = profile.fields;
    std::sort(sortedFields.begin(), sortedFields.end(), 
        [](const ProfileField& a, const ProfileField& b) {
            return a.displayOrder < b.displayOrder;
        });
    
    // Emphasize id and displayName fields
    for (const auto& field : sortedFields)
    {
        // Special emphasis for id and displayName
        if (field.name == "id" || field.name == "displayName")
        {
            prompt << "*** CRITICAL FIELD ***\n";
        }
        prompt << "Field: " << field.name << "\n";
        prompt << "  Display Name: " << field.displayName << "\n";
        prompt << "  Description: " << field.description << "\n";
        prompt << "  Category: " << field.category << "\n";
        prompt << "  Type: ";
        
        switch (field.type)
        {
            case ProfileFieldType::String:
                prompt << "string";
                break;
            case ProfileFieldType::Integer:
                prompt << "integer";
                break;
            case ProfileFieldType::Float:
                prompt << "float";
                break;
            case ProfileFieldType::Boolean:
                prompt << "boolean";
                break;
            case ProfileFieldType::Array:
                prompt << "array";
                break;
            case ProfileFieldType::Object:
                prompt << "object";
                break;
        }
        prompt << "\n";
        
        // Validation rules
        if (field.validation.isRequired)
        {
            prompt << "  REQUIRED: Yes\n";
        }
        else
        {
            prompt << "  REQUIRED: No";
            if (!field.defaultValue.is_null())
            {
                prompt << " (default: " << field.defaultValue.dump() << ")";
            }
            prompt << "\n";
        }
        
        // Type-specific validation
        if (field.type == ProfileFieldType::String)
        {
            if (field.validation.minLength > 0)
                prompt << "  Min Length: " << field.validation.minLength << "\n";
            if (field.validation.maxLength > 0)
                prompt << "  Max Length: " << field.validation.maxLength << "\n";
            if (!field.validation.allowedValues.empty())
            {
                prompt << "  Allowed Values: ";
                for (size_t i = 0; i < field.validation.allowedValues.size(); ++i)
                {
                    if (i > 0) prompt << ", ";
                    prompt << field.validation.allowedValues[i];
                }
                prompt << "\n";
            }
        }
        else if (field.type == ProfileFieldType::Integer || field.type == ProfileFieldType::Float)
        {
            if (field.validation.minValue != 0.0)
                prompt << "  Min Value: " << field.validation.minValue << "\n";
            if (field.validation.maxValue != 0.0)
                prompt << "  Max Value: " << field.validation.maxValue << "\n";
        }
        else if (field.type == ProfileFieldType::Array)
        {
            if (field.validation.minLength > 0)
                prompt << "  Min Elements: " << field.validation.minLength << "\n";
            if (field.validation.maxLength > 0)
                prompt << "  Max Elements: " << field.validation.maxLength << "\n";
        }
        
        // Relationship constraints
        if (!field.validation.relationshipConstraints.empty())
        {
            prompt << "  Relationship Constraints:\n";
            for (const auto& constraint : field.validation.relationshipConstraints)
            {
                prompt << "    - " << constraint.description << "\n";
                prompt << "      (" << field.name << " " << constraint.operator_ 
                       << " " << constraint.targetField << ")\n";
            }
        }
        
        // Custom constraint
        if (!field.validation.customConstraint.empty())
        {
            prompt << "  Custom Constraint: " << field.validation.customConstraint << "\n";
        }
        
        prompt << "\n";
    }
    
    // Add existing IDs to avoid
    if (!existingIds.empty())
    {
        prompt << "\n=== EXISTING ITEM IDs TO AVOID ===\n";
        prompt << "IMPORTANT: Do NOT use these existing item IDs. Generate NEW unique IDs.\n";
        prompt << "Avoid reusing stems; use fresh, novel names, not simple number suffixes.\n\n";
        
        int idCount = 0;
        for (const auto& id : existingIds)
        {
            if (idCount > 0) prompt << ", ";
            prompt << id;
            idCount++;
            if (idCount >= 20) // Limit to first 20
            {
                prompt << " ... (and " << (existingIds.size() - 20) << " more)";
                break;
            }
        }
        prompt << "\n\n";
    }
    
    // Output format instructions
    prompt << "\n=== OUTPUT FORMAT ===\n";
    prompt << "Return a JSON array of " << params.count << " items.\n";
    prompt << "Each item must be a JSON object with all required fields.\n";
    prompt << "Example structure:\n";
    prompt << "[\n";
    prompt << "  {\n";
    
    // Show example with first few fields
    int exampleCount = 0;
    for (const auto& field : sortedFields)
    {
        if (exampleCount >= 5) break;
        prompt << "    \"" << field.name << "\": ";
        switch (field.type)
        {
            case ProfileFieldType::String:
                prompt << "\"example_value\"";
                break;
            case ProfileFieldType::Integer:
                prompt << "0";
                break;
            case ProfileFieldType::Float:
                prompt << "0.0";
                break;
            case ProfileFieldType::Boolean:
                prompt << "false";
                break;
            case ProfileFieldType::Array:
                prompt << "[]";
                break;
            case ProfileFieldType::Object:
                prompt << "{}";
                break;
        }
        prompt << ",\n";
        exampleCount++;
    }
    prompt << "    ...\n";
    prompt << "  }\n";
    prompt << "]\n\n";
    
    prompt << "Generate " << params.count << " unique, creative " << profile.itemTypeName 
           << " items that fit the world context and follow all validation rules.\n";
    
    return prompt.str();
}
