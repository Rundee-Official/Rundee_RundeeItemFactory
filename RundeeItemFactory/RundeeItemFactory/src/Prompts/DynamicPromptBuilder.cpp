/**
 * @file DynamicPromptBuilder.cpp
 * @brief Implementation of dynamic prompt builder
 * @author Haneul Lee (Rundee)
 * @date 2025-12-21
 * @copyright Copyright (c) 2025 Haneul Lee. All rights reserved.
 */

#include "Prompts/DynamicPromptBuilder.h"
#include "Prompts/CustomPreset.h"
#include "Helpers/ItemGenerateParams.h"
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace
{
    std::string BuildExcludeSection(const std::set<std::string>& excludeIds)
    {
        if (excludeIds.empty())
            return {};

        std::string text;
        text += "\nIMPORTANT - Avoid these existing item IDs (do NOT use these):\n";
        int count = 0;
        const int kMaxList = 40;
        for (const auto& id : excludeIds)
        {
            if (count > 0) text += ", ";
            text += id;
            count++;
            if (count >= kMaxList)
            {
                text += " ... (and " + std::to_string(excludeIds.size() - kMaxList) + " more, list truncated)";
                break;
            }
        }
        text += "\nGenerate NEW unique IDs that are different from all existing IDs (assume many more exist). Avoid reusing stems; use fresh, novel names, not simple number suffixes.\n";
        text += "Do NOT generate items that are near-duplicates in concept or function of anything above or of each other in the same batch. Each item must have a distinct idea/material/purpose, not just small adjective changes.\n";
        return text;
    }
}

std::string DynamicPromptBuilder::BuildPromptFromProfile(
    const ItemProfile& profile,
    const FoodGenerateParams& params,
    const CustomPreset& customPreset,
    const std::set<std::string>& excludeIds,
    const std::string& modelName,
    const std::string& generationTimestamp,
    int existingCount)
{
    std::string prompt;
    
    // 1) Add world context (custom context takes priority, then custom preset)
    if (!profile.customContext.empty())
    {
        prompt += "World context:\n";
        prompt += profile.customContext;
        if (profile.customContext.back() != '\n')
            prompt += "\n";
        prompt += "\n";
    }
    else
    {
        prompt += GetPresetFlavorText(customPreset);
    }
    
    // 2) Describe player parameters (if applicable)
    // Use playerSettings from profile if available, otherwise use params
    int maxHunger = profile.playerSettings.maxHunger > 0 ? profile.playerSettings.maxHunger : params.maxHunger;
    int maxThirst = profile.playerSettings.maxThirst > 0 ? profile.playerSettings.maxThirst : params.maxThirst;
    int maxHealth = profile.playerSettings.maxHealth > 0 ? profile.playerSettings.maxHealth : params.maxHealth;
    int maxStamina = profile.playerSettings.maxStamina > 0 ? profile.playerSettings.maxStamina : params.maxStamina;
    int maxWeight = profile.playerSettings.maxWeight > 0 ? profile.playerSettings.maxWeight : params.maxWeight;
    int maxEnergy = profile.playerSettings.maxEnergy > 0 ? profile.playerSettings.maxEnergy : params.maxEnergy;
    
    prompt += "The player has:\n";
    
    prompt += "- maxHunger = " + std::to_string(maxHunger) + "\n";
    prompt += "- maxThirst = " + std::to_string(maxThirst) + "\n";
    prompt += "- maxHealth = " + std::to_string(maxHealth) + "\n";
    prompt += "- maxStamina = " + std::to_string(maxStamina) + "\n";
    prompt += "- maxWeight = " + std::to_string(maxWeight) + " (grams)\n";
    prompt += "- maxEnergy = " + std::to_string(maxEnergy) + "\n\n";
    
    // 3) Task description
    prompt += "Task:\n";
    prompt += "Generate " + std::to_string(params.count) + " " + profile.itemTypeName + " items ";
    prompt += "for the specified world setting.\n";
    
    // Add exclusion list if provided
    prompt += BuildExcludeSection(excludeIds);
    prompt += "\n";
    
    // 4) Define JSON Rules/Schema from profile (CRITICAL - exact structure required)
    prompt += "Rules:\n";
    prompt += "- Use this JSON schema EXACTLY for each item. The structure must match precisely:\n";
    prompt += ProfileToJsonSchemaString(profile);
    prompt += "\n";
    prompt += "CRITICAL: You MUST include ALL fields shown above in the exact structure specified. ";
    prompt += "Field names, types, and nesting must match exactly.\n\n";
    
    // 5) Add validation rules
    std::string validationRules = BuildValidationRules(profile);
    if (!validationRules.empty())
    {
        prompt += validationRules;
        prompt += "\n";
    }
    
    // 6) Output instructions
    prompt += "Output:\n";
    prompt += "- Output ONLY a JSON array of items matching the exact structure above.\n";
    prompt += "- No comments, no extra text, no Markdown, no explanation.\n";
    prompt += "- Ensure each item has all required fields.\n";
    prompt += "- Each item must be a valid JSON object with all specified fields.\n";
    
    return prompt;
}

std::string DynamicPromptBuilder::ProfileToJsonSchemaString(const ItemProfile& profile)
{
    std::stringstream schema;
    schema << "{\n";
    
    // Sort fields by display order, then by name
    std::vector<const ProfileField*> sortedFields;
    for (const auto& field : profile.fields)
    {
        sortedFields.push_back(&field);
    }
    std::sort(sortedFields.begin(), sortedFields.end(), [](const ProfileField* a, const ProfileField* b) {
        if (a->displayOrder != b->displayOrder)
            return a->displayOrder < b->displayOrder;
        return a->name < b->name;
    });
    
    // Group by category for better organization
    std::map<std::string, std::vector<const ProfileField*>> fieldsByCategory;
    for (const auto* field : sortedFields)
    {
        fieldsByCategory[field->category].push_back(field);
    }
    
    bool firstField = true;
    for (const auto& [category, fields] : fieldsByCategory)
    {
        if (!category.empty())
        {
            schema << "  // " << category << " fields\n";
        }
        
        for (const auto* field : fields)
        {
            if (!firstField) schema << ",\n";
            schema << "  " << std::quoted(field->name) << ": " << FieldToSchemaDescription(*field);
            firstField = false;
        }
    }
    
    schema << "\n}";
    return schema.str();
}

std::string DynamicPromptBuilder::FieldToSchemaDescription(const ProfileField& field)
{
    std::stringstream desc;
    
    switch (field.type)
    {
        case ProfileFieldType::String:
            desc << "string";
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
            
        case ProfileFieldType::Integer:
            desc << "integer";
            if (field.validation.minValue != 0.0 || field.validation.maxValue != 0.0)
            {
                desc << " (";
                if (field.validation.minValue != 0.0)
                    desc << "min: " << static_cast<int>(field.validation.minValue);
                if (field.validation.minValue != 0.0 && field.validation.maxValue != 0.0)
                    desc << ", ";
                if (field.validation.maxValue != 0.0)
                    desc << "max: " << static_cast<int>(field.validation.maxValue);
                desc << ")";
            }
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
            
        case ProfileFieldType::Float:
            desc << "float";
            if (field.validation.minValue != 0.0 || field.validation.maxValue != 0.0)
            {
                desc << " (";
                if (field.validation.minValue != 0.0)
                    desc << "min: " << field.validation.minValue;
                if (field.validation.minValue != 0.0 && field.validation.maxValue != 0.0)
                    desc << ", ";
                if (field.validation.maxValue != 0.0)
                    desc << "max: " << field.validation.maxValue;
                desc << ")";
            }
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
            
        case ProfileFieldType::Boolean:
            desc << "boolean";
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
            
        case ProfileFieldType::Array:
            desc << "array";
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
            
        case ProfileFieldType::Object:
            desc << "object";
            if (!field.description.empty())
            {
                desc << " // " << field.description;
            }
            break;
    }
    
    // Add allowed values if specified
    if (!field.validation.allowedValues.empty())
    {
        desc << " (allowed: ";
        for (size_t i = 0; i < field.validation.allowedValues.size(); ++i)
        {
            if (i > 0) desc << " | ";
            desc << std::quoted(field.validation.allowedValues[i]);
        }
        desc << ")";
    }
    
    return desc.str();
}

std::string DynamicPromptBuilder::BuildValidationRules(const ItemProfile& profile)
{
    std::stringstream rules;
    bool hasRules = false;
    
    // First, collect field-level validation rules
    for (const auto& field : profile.fields)
    {
        std::vector<std::string> fieldRules;
        
        if (field.validation.isRequired)
        {
            fieldRules.push_back(field.name + " is REQUIRED");
        }
        
        if (field.type == ProfileFieldType::Integer || field.type == ProfileFieldType::Float)
        {
            if (field.validation.minValue != 0.0)
            {
                fieldRules.push_back(field.name + " must be >= " + std::to_string(field.validation.minValue));
            }
            if (field.validation.maxValue != 0.0)
            {
                fieldRules.push_back(field.name + " must be <= " + std::to_string(field.validation.maxValue));
            }
        }
        
        if (field.type == ProfileFieldType::String)
        {
            if (field.validation.minLength > 0)
            {
                fieldRules.push_back(field.name + " must be at least " + std::to_string(field.validation.minLength) + " characters");
            }
            if (field.validation.maxLength > 0)
            {
                fieldRules.push_back(field.name + " must be at most " + std::to_string(field.validation.maxLength) + " characters");
            }
        }
        
        // Add relationship constraints
        for (const auto& constraint : field.validation.relationshipConstraints)
        {
            std::string constraintText = field.name + " " + constraint.operator_ + " " + constraint.targetField;
            if (constraint.offset != 0.0)
            {
                if (constraint.offset > 0)
                    constraintText += " + " + std::to_string(constraint.offset);
                else
                    constraintText += " - " + std::to_string(std::abs(constraint.offset));
            }
            if (!constraint.description.empty())
            {
                constraintText += " (" + constraint.description + ")";
            }
            fieldRules.push_back(constraintText);
        }
        
        // Add custom constraint if specified
        if (!field.validation.customConstraint.empty())
        {
            fieldRules.push_back(field.name + ": " + field.validation.customConstraint);
        }
        
        if (!fieldRules.empty())
        {
            if (hasRules) rules << "\n";
            for (const auto& rule : fieldRules)
            {
                rules << "- " << rule << "\n";
            }
            hasRules = true;
        }
    }
    
    return rules.str();
}

std::string DynamicPromptBuilder::GetPresetFlavorText(const CustomPreset& customPreset)
{
    return CustomPresetManager::GetPresetFlavorText(customPreset);
}

std::string DynamicPromptBuilder::BuildExcludeSection(const std::set<std::string>& excludeIds)
{
    return ::BuildExcludeSection(excludeIds);
}

