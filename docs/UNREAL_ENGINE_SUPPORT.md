# Unreal Engine Support Guide

## Overview

This document outlines how to implement the same UI features in Unreal Engine that are available in Unity. The core C++ logic is shared, but UI implementation differs between engines.

## Architecture

### Shared Components (C++)
- **ItemGenerator.cpp**: Core item generation logic
- **ItemJsonParser.cpp**: JSON parsing for all item types
- **OllamaClient.cpp**: LLM API communication
- **Registry Management**: ID tracking and duplicate prevention

### Engine-Specific UI

#### Unity
- Uses `EditorWindow` and IMGUI
- Files: `ItemFactoryWindow.cs`, `RegistryManagerWindow.cs`, etc.
- Location: `UnityRundeeItemFactory/Assets/RundeeItemFactory/Editor/`

#### Unreal Engine
- Uses **Slate UI** or **Editor Utility Widgets** (UMG)
- Recommended: Editor Utility Widgets for easier development
- Location: `UnrealRundeeItemFactory/Source/RundeeItemFactoryEditor/`

## Feature Mapping

### 1. Registry Manager UI

**Unity Implementation:**
- `RegistryManagerWindow.cs` - EditorWindow with IMGUI
- Features: View, search, delete IDs, clear registry, export/import

**Unreal Implementation:**
```cpp
// Unreal: Editor Utility Widget (UMG-based)
UCLASS()
class RUNDEEITEMFACTORYEDITOR_API URegistryManagerWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()
    
public:
    UPROPERTY(meta = (BindWidget))
    class UComboBoxString* ItemTypeComboBox;
    
    UPROPERTY(meta = (BindWidget))
    class UEditableTextBox* SearchBox;
    
    UPROPERTY(meta = (BindWidget))
    class UListView* IDListView;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* DeleteSelectedButton;
    
    UPROPERTY(meta = (BindWidget))
    class UButton* ClearRegistryButton;
    
    // Load registry from JSON file
    UFUNCTION(BlueprintCallable)
    void LoadRegistry(const FString& ItemType);
    
    // Delete selected IDs
    UFUNCTION(BlueprintCallable)
    void DeleteSelectedIDs();
    
    // Clear entire registry
    UFUNCTION(BlueprintCallable)
    void ClearRegistry();
};
```

**Alternative: Slate UI (More Control)**
```cpp
class FRegistryManagerWindow : public FTabManager
{
public:
    void Construct(const FArguments& InArgs);
    
    TSharedRef<SWidget> MakeItemTypeSelector();
    TSharedRef<SWidget> MakeIDList();
    TSharedRef<SWidget> MakeActionButtons();
    
    void OnDeleteSelected();
    void OnClearRegistry();
};
```

### 2. Item Preview & Edit Window

**Unity:** Custom EditorWindow with property fields

**Unreal:** 
- Use **Details Panel** customization
- Or create custom **Editor Utility Widget** with property editors
- Leverage Unreal's reflection system for automatic property binding

```cpp
UCLASS()
class UItemPreviewWidget : public UEditorUtilityWidget
{
    // Use UPROPERTY with meta = (BindWidget) for automatic binding
    // Use USTRUCT for item data with UPROPERTY() for serialization
};
```

### 3. Batch Job Manager

**Unity:** List<BatchItem> with EditorGUILayout

**Unreal:**
```cpp
USTRUCT()
struct FBatchJob
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    EItemType ItemType;
    
    UPROPERTY(EditAnywhere)
    int32 Count;
    
    UPROPERTY(EditAnywhere)
    FString OutputPath;
};

UCLASS()
class UBatchJobManagerWidget : public UEditorUtilityWidget
{
    UPROPERTY(meta = (BindWidget))
    class UListView* JobListView;
    
    UPROPERTY()
    TArray<FBatchJob> Jobs;
    
    UFUNCTION(BlueprintCallable)
    void AddJob(const FBatchJob& Job);
    
    UFUNCTION(BlueprintCallable)
    void ExecuteBatch();
};
```

### 4. Error Handling & User Feedback

**Unity:** EditorUtility.DisplayDialog, Debug.Log

**Unreal:**
```cpp
// Show message dialog
FMessageDialog::Open(EAppMsgType::Ok, 
    FText::FromString(TEXT("Error message here")));

// Log to output log
UE_LOG(LogRundeeItemFactory, Error, TEXT("Error: %s"), *ErrorMessage);

// Show notification
FNotificationInfo Info(FText::FromString(TEXT("Success!")));
Info.bFireAndForget = true;
FSlateNotificationManager::Get().AddNotification(Info);
```

### 5. Statistics Dashboard

**Unity:** EditorGUILayout with charts (third-party or custom)

**Unreal:**
- Use **Slate Charts** or **UMG Widgets** with custom drawing
- Leverage Unreal's **Data Visualization** plugins
- Or use simple text-based statistics display

### 6. Backup & Version Management

**Shared Logic (C++):**
```cpp
class FVersionManager
{
public:
    static bool CreateBackup(const FString& FilePath);
    static bool RestoreBackup(const FString& BackupPath);
    static TArray<FString> GetVersionHistory(const FString& FilePath);
};
```

**Unreal UI:** Editor Utility Widget with file browser

### 7. User Guide & Tutorial

**Unity:** HelpBox, tooltips, embedded documentation

**Unreal:**
- Use **Tooltip** system in Slate/UMG
- Create **Help Window** as Editor Utility Widget
- Use **Rich Text** widgets for formatted documentation

### 8. Preset Customization UI

**Shared:** JSON-based preset files

**Unreal:**
```cpp
USTRUCT()
struct FItemPreset
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    FString Name;
    
    UPROPERTY(EditAnywhere)
    FString Context;
    
    // ... other preset properties
};

UCLASS()
class UPresetManagerWidget : public UEditorUtilityWidget
{
    UPROPERTY(meta = (BindWidget))
    class UListView* PresetListView;
    
    UFUNCTION(BlueprintCallable)
    void CreatePreset(const FItemPreset& Preset);
    
    UFUNCTION(BlueprintCallable)
    void EditPreset(int32 Index);
};
```

### 9. Quality Check UI

**Shared:** C++ QualityChecker logic

**Unreal:** Display results in Editor Utility Widget with color-coded warnings/errors

### 10. Performance & Usability

**Cancellation:**
- Unity: Process.Kill()
- Unreal: Async task cancellation with `FCancelableAsyncTask`

**Partial Save:**
- Both: Write intermediate results to temp files

### 11. Export/Import

**Shared:** JSON format (already implemented)

**Unreal:** Use Unreal's file system API
```cpp
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"

bool ExportToCSV(const FString& FilePath, const TArray<FItemData>& Items)
{
    FString CSVContent;
    // Build CSV string
    return FFileHelper::SaveStringToFile(CSVContent, *FilePath);
}
```

## Implementation Steps

### Phase 1: Core Setup
1. Create Unreal plugin/module structure
2. Port shared C++ code (minimal changes needed)
3. Set up build system

### Phase 2: Basic UI
1. Create Editor Utility Widget base
2. Implement Registry Manager
3. Implement Item Factory Window

### Phase 3: Advanced Features
1. Batch job manager
2. Statistics dashboard
3. Preset manager

### Phase 4: Polish
1. Error handling
2. User guides
3. Performance optimization

## Key Differences

| Feature | Unity | Unreal |
|---------|-------|--------|
| UI Framework | IMGUI (EditorWindow) | Slate / UMG (Editor Utility Widget) |
| Property Editing | SerializedProperty | UPROPERTY reflection |
| File Dialogs | EditorUtility | FDesktopPlatformModule |
| Logging | Debug.Log | UE_LOG |
| Async Tasks | Coroutines / async | AsyncTask / Threading |
| Notifications | EditorUtility.DisplayDialog | FNotificationInfo |

## Code Sharing Strategy

1. **Keep C++ logic in shared location** (RundeeItemFactory/)
2. **Engine-specific UI in separate folders:**
   - `UnityRundeeItemFactory/` - Unity C# scripts
   - `UnrealRundeeItemFactory/` - Unreal C++/Blueprint

3. **Use JSON for data exchange** (already implemented)
4. **Shared configuration files** (config/, prompts/, Registry/)

## Testing Strategy

1. Test C++ logic independently (unit tests)
2. Test UI in each engine separately
3. Verify JSON compatibility between engines
4. Test registry file format compatibility

## Future Enhancements

- **Plugin System**: Allow custom item types via plugins
- **Remote API**: REST API for web-based UI
- **CLI Tool**: Command-line interface (already exists, can be enhanced)



