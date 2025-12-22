using System;
using System.IO;
using System.Text.RegularExpressions;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Input validation utilities for Unity Editor windows
/// </summary>
public static class InputValidator
{
    /// <summary>
    /// Validation result with error message
    /// </summary>
    public class ValidationResult
    {
        public bool IsValid { get; set; }
        public string ErrorMessage { get; set; }
        public string Suggestion { get; set; }

        public ValidationResult(bool isValid, string errorMessage = "", string suggestion = "")
        {
            IsValid = isValid;
            ErrorMessage = errorMessage;
            Suggestion = suggestion;
        }

        public static ValidationResult Valid() => new ValidationResult(true);
        public static ValidationResult Invalid(string error, string suggestion = "") => new ValidationResult(false, error, suggestion);
    }

    /// <summary>
    /// Validate item count
    /// </summary>
    public static ValidationResult ValidateItemCount(int count)
    {
        if (count < 1)
        {
            return ValidationResult.Invalid(
                "Item count must be at least 1",
                "Enter a number between 1 and 1000"
            );
        }

        if (count > 1000)
        {
            return ValidationResult.Invalid(
                "Item count is too large (max 1000)",
                "For large batches, use Batch Job Manager instead"
            );
        }

        if (count > 100)
        {
            return ValidationResult.Invalid(
                "Large item counts may take a long time",
                "Consider using smaller batches (10-50 items) for better performance"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate file path
    /// </summary>
    public static ValidationResult ValidateFilePath(string path, bool mustExist = false, string extension = null)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return ValidationResult.Invalid(
                "File path cannot be empty",
                "Please select a file path"
            );
        }

        // Check for invalid characters
        char[] invalidChars = Path.GetInvalidPathChars();
        foreach (char c in invalidChars)
        {
            if (path.Contains(c))
            {
                return ValidationResult.Invalid(
                    $"File path contains invalid character: '{c}'",
                    "Remove invalid characters from the path"
                );
            }
        }

        // Check extension
        if (!string.IsNullOrEmpty(extension))
        {
            string actualExtension = Path.GetExtension(path);
            if (!actualExtension.Equals(extension, StringComparison.OrdinalIgnoreCase))
            {
                return ValidationResult.Invalid(
                    $"File must have {extension} extension",
                    $"Change file extension to {extension}"
                );
            }
        }

        // Check if file exists (if required)
        if (mustExist && !File.Exists(path))
        {
            return ValidationResult.Invalid(
                "File does not exist",
                "Please select an existing file or create it first"
            );
        }

        // Check directory exists
        string directory = Path.GetDirectoryName(path);
        if (!string.IsNullOrEmpty(directory) && !Directory.Exists(directory))
        {
            return ValidationResult.Invalid(
                "Directory does not exist",
                "The directory will be created automatically, or select a different path"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate model name
    /// </summary>
    public static ValidationResult ValidateModelName(string modelName)
    {
        if (string.IsNullOrWhiteSpace(modelName))
        {
            return ValidationResult.Invalid(
                "Model name cannot be empty",
                "Enter a model name (e.g., 'llama3', 'mistral:7b')"
            );
        }

        // Check for valid characters (alphanumeric, colon, dash, underscore)
        if (!Regex.IsMatch(modelName, @"^[a-zA-Z0-9:_\-]+$"))
        {
            return ValidationResult.Invalid(
                "Model name contains invalid characters",
                "Use only letters, numbers, colons, dashes, and underscores"
            );
        }

        if (modelName.Length > 100)
        {
            return ValidationResult.Invalid(
                "Model name is too long",
                "Model names should be less than 100 characters"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate preset path
    /// </summary>
    public static ValidationResult ValidatePresetPath(string path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return ValidationResult.Invalid(
                "Preset path cannot be empty",
                "Please select a preset file"
            );
        }

        if (!File.Exists(path))
        {
            return ValidationResult.Invalid(
                "Preset file does not exist",
                "Please select an existing preset JSON file"
            );
        }

        if (!path.EndsWith(".json", StringComparison.OrdinalIgnoreCase))
        {
            return ValidationResult.Invalid(
                "Preset file must be a JSON file",
                "Select a file with .json extension"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate executable path
    /// </summary>
    public static ValidationResult ValidateExecutablePath(string path)
    {
        if (string.IsNullOrWhiteSpace(path))
        {
            return ValidationResult.Invalid(
                "Executable path cannot be empty",
                "Please set the path to RundeeItemFactory.exe"
            );
        }

        if (!File.Exists(path))
        {
            return ValidationResult.Invalid(
                "Executable file does not exist",
                "Build the project or set the correct path to RundeeItemFactory.exe"
            );
        }

        if (!path.EndsWith(".exe", StringComparison.OrdinalIgnoreCase))
        {
            return ValidationResult.Invalid(
                "File must be an executable (.exe)",
                "Select the RundeeItemFactory.exe file"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate numeric range
    /// </summary>
    public static ValidationResult ValidateRange(int value, int min, int max, string fieldName)
    {
        if (value < min || value > max)
        {
            return ValidationResult.Invalid(
                $"{fieldName} must be between {min} and {max}",
                $"Enter a value between {min} and {max}"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Validate output directory is writable
    /// </summary>
    public static ValidationResult ValidateOutputDirectory(string filePath)
    {
        if (string.IsNullOrWhiteSpace(filePath))
        {
            return ValidationResult.Valid(); // Will be validated elsewhere
        }

        try
        {
            string directory = Path.GetDirectoryName(filePath);
            if (string.IsNullOrEmpty(directory))
            {
                return ValidationResult.Invalid(
                    "Invalid file path",
                    "Please provide a full file path"
                );
            }

            // Try to create directory if it doesn't exist
            if (!Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }

            // Test write permissions
            string testFile = Path.Combine(directory, ".write_test");
            try
            {
                File.WriteAllText(testFile, "test");
                File.Delete(testFile);
            }
            catch (UnauthorizedAccessException)
            {
                return ValidationResult.Invalid(
                    "Directory is not writable",
                    "Check directory permissions or select a different location"
                );
            }
            catch (Exception ex)
            {
                return ValidationResult.Invalid(
                    $"Cannot write to directory: {ex.Message}",
                    "Check directory permissions"
                );
            }
        }
        catch (Exception ex)
        {
            return ValidationResult.Invalid(
                $"Invalid directory: {ex.Message}",
                "Select a valid directory path"
            );
        }

        return ValidationResult.Valid();
    }

    /// <summary>
    /// Display validation error in GUI
    /// </summary>
    public static void DisplayValidationError(ValidationResult result)
    {
        if (!result.IsValid)
        {
            EditorGUILayout.HelpBox(result.ErrorMessage, MessageType.Error);
            if (!string.IsNullOrEmpty(result.Suggestion))
            {
                EditorGUILayout.HelpBox($"Suggestion: {result.Suggestion}", MessageType.Info);
            }
        }
    }
}






