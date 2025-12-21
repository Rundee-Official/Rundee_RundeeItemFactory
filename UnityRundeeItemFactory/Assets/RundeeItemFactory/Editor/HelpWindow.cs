using UnityEditor;
using UnityEngine;

/// <summary>
/// Help window with user guide and tutorials
/// </summary>
public class HelpWindow : EditorWindow
{
    private Vector2 scrollPosition;
    private int selectedTab = 0;
    private string[] tabs = { "Getting Started", "Presets", "Troubleshooting", "FAQ" };
    
    [MenuItem("Tools/Rundee/Item Factory/Help/Help & Tutorial", false, 5000)]
    public static void ShowWindow()
    {
        var window = GetWindow<HelpWindow>("Help & Tutorial");
        window.minSize = new Vector2(600, 500);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Help & Tutorial", EditorStyles.boldLabel);
        
        EditorGUILayout.Space();
        
        // Tabs
        selectedTab = GUILayout.Toolbar(selectedTab, tabs);
        
        EditorGUILayout.Space();
        
        scrollPosition = EditorGUILayout.BeginScrollView(scrollPosition);
        
        switch (selectedTab)
        {
            case 0:
                DrawGettingStarted();
                break;
            case 1:
                DrawPresets();
                break;
            case 2:
                DrawTroubleshooting();
                break;
            case 3:
                DrawFAQ();
                break;
        }
        
        EditorGUILayout.EndScrollView();
    }
    
    private void DrawGettingStarted()
    {
        EditorGUILayout.LabelField("Getting Started", EditorStyles.boldLabel);
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Step 1: Install Ollama", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "1. Download Ollama from https://ollama.ai\n" +
            "2. Install and start Ollama\n" +
            "3. Pull a model: 'ollama pull llama3'\n" +
            "4. Verify installation: 'ollama list'",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Step 2: Configure Executable", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "1. Open Item Factory Window (Tools > Rundee > Item Factory > Item Factory Window)\n" +
            "2. Set the executable path to RundeeItemFactory.exe\n" +
            "3. Verify the path is correct",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Step 3: Generate Your First Items", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "1. Select item type (Food, Drink, etc.)\n" +
            "2. Choose a preset (Default, Forest, etc.)\n" +
            "3. Set count (start with 10 for testing)\n" +
            "4. Click 'Generate Items'\n" +
            "5. Wait for generation to complete\n" +
            "6. Items will be automatically imported if 'Auto Import' is enabled",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Step 4: Manage Your Items", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "1. Use Item Manager to browse and delete items\n" +
            "2. Use Registry Manager to manage ID registries\n" +
            "3. Use Statistics Dashboard to analyze balance",
            MessageType.Info);
    }
    
    private void DrawPresets()
    {
        EditorGUILayout.LabelField("Presets Guide", EditorStyles.boldLabel);
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Default", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Generic survival environment with moderate resources. " +
            "Items should feel simple and grounded, not magical or high-tech.",
            MessageType.None);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Forest", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Temperate forest environment with natural resources, wild foods, " +
            "and organic materials. Items include berries, nuts, roots, and simple cooked meals.",
            MessageType.None);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Desert", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Harsh desert environment with scarce resources, preserved foods, " +
            "and heat-resistant materials. Food items are low in quantity but efficient.",
            MessageType.None);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Coast", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Coastal environment with seafood, fresh water sources, and marine materials. " +
            "Items include coconuts, fish, shellfish, and tropical fruits.",
            MessageType.None);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("City", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Urban environment with processed foods, manufactured items, and modern weapons. " +
            "Items include canned goods, energy drinks, and advanced weaponry.",
            MessageType.None);
    }
    
    private void DrawTroubleshooting()
    {
        EditorGUILayout.LabelField("Troubleshooting", EditorStyles.boldLabel);
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Ollama Not Found", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Problem: 'Ollama not found' error\n\n" +
            "Solution:\n" +
            "1. Ensure Ollama is installed\n" +
            "2. Start Ollama application\n" +
            "3. Verify with 'ollama list' command\n" +
            "4. Check firewall settings",
            MessageType.Warning);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Connection Failed", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Problem: Cannot connect to Ollama\n\n" +
            "Solution:\n" +
            "1. Check if Ollama is running\n" +
            "2. Verify host and port in config (default: localhost:11434)\n" +
            "3. Check network/firewall settings\n" +
            "4. Try restarting Ollama",
            MessageType.Warning);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Model Not Found", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Problem: Model not available\n\n" +
            "Solution:\n" +
            "1. Pull the model: 'ollama pull [model_name]'\n" +
            "2. List available models: 'ollama list'\n" +
            "3. Use a different model",
            MessageType.Warning);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Invalid JSON", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Problem: Generated JSON is invalid\n\n" +
            "Solution:\n" +
            "1. Try generating again\n" +
            "2. Check LLM model quality\n" +
            "3. Review prompt templates\n" +
            "4. Try a different model",
            MessageType.Warning);
    }
    
    private void DrawFAQ()
    {
        EditorGUILayout.LabelField("Frequently Asked Questions", EditorStyles.boldLabel);
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Q: How many items should I generate?", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "A: Start with 10-20 items for testing. For production, 50-100 items per type " +
            "is recommended. More items take longer to generate but provide better variety.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Q: Can I edit generated items?", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "A: Yes! Use the Item Preview window to preview and edit items before importing. " +
            "After importing, you can edit ScriptableObjects directly in Unity.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Q: What if I get duplicate IDs?", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "A: The system automatically prevents duplicates using ID registries. " +
            "If you see duplicates, use the Registry Manager to clean up.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Q: How do I change the LLM model?", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "A: In the Item Factory Window, select a different model from the dropdown " +
            "or enter a custom model name. Make sure the model is pulled in Ollama first.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        EditorGUILayout.LabelField("Q: Can I use my own prompts?", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "A: Yes! Edit the prompt files in the prompts/ folder. " +
            "You can also create preset-specific prompts (e.g., food_forest.txt).",
            MessageType.Info);
    }
}

