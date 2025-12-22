using System;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Centralized error handling and user feedback system
/// </summary>
public static class ErrorHandler
{
    public enum ErrorType
    {
        OllamaNotRunning,
        OllamaConnectionFailed,
        ModelNotFound,
        InvalidJSON,
        FileNotFound,
        PermissionDenied,
        GenerationFailed,
        ImportFailed,
        Unknown
    }
    
    private static Dictionary<ErrorType, ErrorInfo> errorDatabase = new Dictionary<ErrorType, ErrorInfo>
    {
        {
            ErrorType.OllamaNotRunning,
            new ErrorInfo
            {
                Title = "Ollama Not Running",
                Message = "Ollama service is not running. Please start Ollama before generating items.",
                Solution = "1. Open Ollama application\n2. Or run 'ollama serve' in terminal\n3. Verify Ollama is running: 'ollama list'",
                Severity = ErrorSeverity.Critical
            }
        },
        {
            ErrorType.OllamaConnectionFailed,
            new ErrorInfo
            {
                Title = "Connection Failed",
                Message = "Failed to connect to Ollama server.",
                Solution = "1. Check if Ollama is running\n2. Verify host and port in config (default: localhost:11434)\n3. Check firewall settings",
                Severity = ErrorSeverity.Critical
            }
        },
        {
            ErrorType.ModelNotFound,
            new ErrorInfo
            {
                Title = "Model Not Found",
                Message = "The specified LLM model is not available.",
                Solution = "1. Pull the model: 'ollama pull [model_name]'\n2. List available models: 'ollama list'\n3. Use a different model",
                Severity = ErrorSeverity.Error
            }
        },
        {
            ErrorType.InvalidJSON,
            new ErrorInfo
            {
                Title = "Invalid JSON",
                Message = "The generated JSON is invalid or malformed.",
                Solution = "1. Try generating again\n2. Check LLM model quality\n3. Review prompt templates",
                Severity = ErrorSeverity.Warning
            }
        },
        {
            ErrorType.FileNotFound,
            new ErrorInfo
            {
                Title = "File Not Found",
                Message = "The specified file path does not exist.",
                Solution = "1. Check file path\n2. Ensure file exists\n3. Check file permissions",
                Severity = ErrorSeverity.Error
            }
        },
        {
            ErrorType.PermissionDenied,
            new ErrorInfo
            {
                Title = "Permission Denied",
                Message = "Access to the file or directory is denied.",
                Solution = "1. Check file permissions\n2. Run Unity as administrator if needed\n3. Check antivirus settings",
                Severity = ErrorSeverity.Error
            }
        },
        {
            ErrorType.GenerationFailed,
            new ErrorInfo
            {
                Title = "Generation Failed",
                Message = "Item generation failed. Some items may have been created.",
                Solution = "1. Check log for details\n2. Try generating again\n3. Reduce item count if memory issues",
                Severity = ErrorSeverity.Warning
            }
        },
        {
            ErrorType.ImportFailed,
            new ErrorInfo
            {
                Title = "Import Failed",
                Message = "Failed to import items into Unity.",
                Solution = "1. Check JSON file validity\n2. Ensure correct item type selected\n3. Check Unity console for details",
                Severity = ErrorSeverity.Error
            }
        }
    };
    
    public enum ErrorSeverity
    {
        Info,
        Warning,
        Error,
        Critical
    }
    
    private class ErrorInfo
    {
        public string Title;
        public string Message;
        public string Solution;
        public ErrorSeverity Severity;
    }
    
    public static void ShowError(ErrorType errorType, string additionalInfo = "")
    {
        if (!errorDatabase.ContainsKey(errorType))
        {
            errorType = ErrorType.Unknown;
        }
        
        var error = errorDatabase[errorType];
        string fullMessage = error.Message;
        
        if (!string.IsNullOrEmpty(additionalInfo))
        {
            fullMessage += $"\n\nDetails: {additionalInfo}";
        }
        
        fullMessage += $"\n\nSolution:\n{error.Solution}";
        
        EditorUtility.DisplayDialog(error.Title, fullMessage, "OK");
        Debug.LogError($"[ErrorHandler] {error.Title}: {fullMessage}");
    }

    /// <summary>
    /// Show error with custom title, message, and solution
    /// </summary>
    public static void ShowError(string title, string message, string solution = "")
    {
        string fullMessage = message;
        if (!string.IsNullOrEmpty(solution))
        {
            fullMessage += $"\n\nSolution:\n{solution}";
        }
        
        EditorUtility.DisplayDialog(title, fullMessage, "OK");
        Debug.LogError($"[ErrorHandler] {title}: {fullMessage}");
    }
    
    public static void ShowSuccess(string message)
    {
        EditorUtility.DisplayDialog("Success", message, "OK");
        Debug.Log($"[ErrorHandler] Success: {message}");
    }
    
    public static void ShowInfo(string title, string message)
    {
        EditorUtility.DisplayDialog(title, message, "OK");
        Debug.Log($"[ErrorHandler] Info: {title} - {message}");
    }
    
    public static bool ShowConfirmation(string title, string message)
    {
        return EditorUtility.DisplayDialog(title, message, "Yes", "No");
    }
}

