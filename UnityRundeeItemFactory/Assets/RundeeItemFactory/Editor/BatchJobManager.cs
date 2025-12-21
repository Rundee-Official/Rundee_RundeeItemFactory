using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEngine;

/// <summary>
/// Batch job manager for queuing and executing multiple item generation tasks
/// </summary>
/// <remarks>
/// Provides functionality to:
/// - Queue multiple generation jobs
/// - Execute jobs sequentially or in parallel
/// - Track job history
/// - Retry failed jobs
/// 
/// Can be extended for Unreal Engine using async tasks and job queues.
/// </remarks>
[System.Serializable]
public class BatchJob
{
    public string id;
    public ItemType itemType;
    public int count;
    public string outputPath;
    public PresetType preset;
    public string model;
    public DateTime createdAt;
    public DateTime? completedAt;
    public JobStatus status;
    public string errorMessage;
    public int retryCount;
    
    public enum JobStatus
    {
        Pending,
        Running,
        Completed,
        Failed,
        Cancelled
    }
}

public class BatchJobManager : ScriptableObject
{
    private static BatchJobManager instance;
    private const string SAVE_PATH = "Assets/RundeeItemFactory/BatchJobs.asset";
    
    public List<BatchJob> jobs = new List<BatchJob>();
    public List<BatchJob> history = new List<BatchJob>();
    
    public static BatchJobManager Instance
    {
        get
        {
            if (instance == null)
            {
                instance = AssetDatabase.LoadAssetAtPath<BatchJobManager>(SAVE_PATH);
                if (instance == null)
                {
                    instance = CreateInstance<BatchJobManager>();
                    string directory = Path.GetDirectoryName(SAVE_PATH);
                    if (!Directory.Exists(directory))
                    {
                        Directory.CreateDirectory(directory);
                    }
                    AssetDatabase.CreateAsset(instance, SAVE_PATH);
                    AssetDatabase.SaveAssets();
                }
            }
            return instance;
        }
    }
    
    public void AddJob(BatchJob job)
    {
        if (string.IsNullOrEmpty(job.id))
        {
            job.id = Guid.NewGuid().ToString();
        }
        job.createdAt = DateTime.Now;
        job.status = BatchJob.JobStatus.Pending;
        jobs.Add(job);
        EditorUtility.SetDirty(this);
        AssetDatabase.SaveAssets();
    }
    
    public void RemoveJob(BatchJob job)
    {
        jobs.Remove(job);
        EditorUtility.SetDirty(this);
        AssetDatabase.SaveAssets();
    }
    
    public void MoveToHistory(BatchJob job)
    {
        jobs.Remove(job);
        history.Add(job);
        
        // Keep only last 100 history items
        if (history.Count > 100)
        {
            history.RemoveAt(0);
        }
        
        EditorUtility.SetDirty(this);
        AssetDatabase.SaveAssets();
    }
    
    public void ClearHistory()
    {
        history.Clear();
        EditorUtility.SetDirty(this);
        AssetDatabase.SaveAssets();
    }
}

/// <summary>
/// Unity Editor window for managing batch generation jobs
/// </summary>
public class BatchJobManagerWindow : EditorWindow
{
    private Vector2 jobScrollPosition;
    private Vector2 historyScrollPosition;
    private bool showHistory = false;
    private BatchJob selectedJob = null;
    
    [MenuItem("Tools/Rundee/Item Factory/Tools/Batch Job Manager", false, 3010)]
    public static void ShowWindow()
    {
        var window = GetWindow<BatchJobManagerWindow>("Batch Job Manager");
        window.minSize = new Vector2(700, 500);
        window.Show();
    }
    
    private void OnGUI()
    {
        EditorGUILayout.Space();
        EditorGUILayout.LabelField("Batch Job Manager", EditorStyles.boldLabel);
        EditorGUILayout.HelpBox(
            "Queue multiple item generation jobs and execute them sequentially. " +
            "Failed jobs can be retried individually.",
            MessageType.Info);
        
        EditorGUILayout.Space();
        
        // Add New Job
        if (GUILayout.Button("Add New Job", GUILayout.Height(30)))
        {
            ShowAddJobDialog();
        }
        
        EditorGUILayout.Space();
        
        // Job List
        EditorGUILayout.LabelField("Pending Jobs", EditorStyles.boldLabel);
        jobScrollPosition = EditorGUILayout.BeginScrollView(jobScrollPosition, GUILayout.Height(200));
        
        var manager = BatchJobManager.Instance;
        foreach (var job in manager.jobs.ToList())
        {
            DrawJobItem(job);
        }
        
        EditorGUILayout.EndScrollView();
        
        EditorGUILayout.Space();
        
        // Actions
        EditorGUILayout.BeginHorizontal();
        if (GUILayout.Button("Execute All", GUILayout.Height(30)))
        {
            ExecuteAllJobs();
        }
        if (GUILayout.Button("Clear Completed", GUILayout.Height(30)))
        {
            ClearCompletedJobs();
        }
        EditorGUILayout.EndHorizontal();
        
        EditorGUILayout.Space();
        
        // History
        showHistory = EditorGUILayout.Foldout(showHistory, "Job History", true);
        if (showHistory)
        {
            historyScrollPosition = EditorGUILayout.BeginScrollView(historyScrollPosition, GUILayout.Height(150));
            
            foreach (var job in manager.history)
            {
                DrawHistoryItem(job);
            }
            
            EditorGUILayout.EndScrollView();
            
            if (GUILayout.Button("Clear History", GUILayout.Width(120)))
            {
                if (EditorUtility.DisplayDialog("Clear History", "Clear all job history?", "Clear", "Cancel"))
                {
                    manager.ClearHistory();
                }
            }
        }
    }
    
    private void DrawJobItem(BatchJob job)
    {
        EditorGUILayout.BeginVertical(EditorStyles.helpBox);
        EditorGUILayout.BeginHorizontal();
        
        EditorGUILayout.LabelField($"{job.itemType} x {job.count}", GUILayout.Width(150));
        EditorGUILayout.LabelField($"Status: {job.status}", GUILayout.Width(120));
        EditorGUILayout.LabelField($"Preset: {job.preset}", GUILayout.Width(100));
        
        GUILayout.FlexibleSpace();
        
        if (job.status == BatchJob.JobStatus.Pending)
        {
            if (GUILayout.Button("Execute", GUILayout.Width(80)))
            {
                ExecuteJob(job);
            }
        }
        else if (job.status == BatchJob.JobStatus.Failed)
        {
            if (GUILayout.Button("Retry", GUILayout.Width(80)))
            {
                RetryJob(job);
            }
        }
        
        if (GUILayout.Button("Remove", GUILayout.Width(80)))
        {
            BatchJobManager.Instance.RemoveJob(job);
        }
        
        EditorGUILayout.EndHorizontal();
        
        if (!string.IsNullOrEmpty(job.errorMessage))
        {
            EditorGUILayout.HelpBox($"Error: {job.errorMessage}", MessageType.Error);
        }
        
        EditorGUILayout.EndVertical();
    }
    
    private void DrawHistoryItem(BatchJob job)
    {
        EditorGUILayout.BeginHorizontal();
        EditorGUILayout.LabelField($"{job.itemType} x {job.count}", GUILayout.Width(150));
        EditorGUILayout.LabelField($"Status: {job.status}", GUILayout.Width(120));
        if (job.completedAt.HasValue)
        {
            EditorGUILayout.LabelField($"Completed: {job.completedAt.Value:yyyy-MM-dd HH:mm:ss}", GUILayout.Width(200));
        }
        EditorGUILayout.EndHorizontal();
    }
    
    private void ShowAddJobDialog()
    {
        // Create a simple dialog using EditorUtility
        // In a real implementation, you might want a more sophisticated dialog
        var job = new BatchJob
        {
            itemType = ItemType.Food,
            count = 10,
            preset = PresetType.Default,
            model = "llama3"
        };
        
        BatchJobManager.Instance.AddJob(job);
        Debug.Log("[BatchJobManager] Job added. Use Item Factory Window to configure job details.");
    }
    
    private void ExecuteJob(BatchJob job)
    {
        job.status = BatchJob.JobStatus.Running;
        EditorUtility.SetDirty(BatchJobManager.Instance);
        
        // Call actual generation process
        try
        {
            PresetType? preset = job.preset;

            bool started = ItemFactoryWindow.GenerateItemsAsync(
                job.itemType,
                job.count,
                job.outputPath,
                job.model ?? "llama3",
                preset,
                null, // customPresetPath - could be added to BatchJob if needed
                (success, message) =>
                {
                    job.status = success ? BatchJob.JobStatus.Completed : BatchJob.JobStatus.Failed;
                    job.completedAt = DateTime.Now;
                    if (!success)
                    {
                        job.errorMessage = message;
                        job.retryCount++;
                    }
                    BatchJobManager.Instance.MoveToHistory(job);
                    EditorUtility.SetDirty(BatchJobManager.Instance);
                    UnityEngine.Debug.Log($"[BatchJobManager] Job {job.id} {(success ? "completed" : "failed")}: {message}");
                }
            );

            if (!started)
            {
                job.status = BatchJob.JobStatus.Failed;
                job.errorMessage = "Failed to start generation";
                job.retryCount++;
                EditorUtility.SetDirty(BatchJobManager.Instance);
            }
        }
        catch (Exception ex)
        {
            job.status = BatchJob.JobStatus.Failed;
            job.errorMessage = ex.Message;
            job.retryCount++;
            EditorUtility.SetDirty(BatchJobManager.Instance);
            
            Debug.LogError($"[BatchJobManager] Job {job.id} failed: {ex.Message}");
        }
    }
    
    private void ExecuteAllJobs()
    {
        var manager = BatchJobManager.Instance;
        var pendingJobs = manager.jobs.Where(j => j.status == BatchJob.JobStatus.Pending).ToList();
        
        foreach (var job in pendingJobs)
        {
            ExecuteJob(job);
        }
    }
    
    private void RetryJob(BatchJob job)
    {
        job.status = BatchJob.JobStatus.Pending;
        job.errorMessage = "";
        EditorUtility.SetDirty(BatchJobManager.Instance);
        ExecuteJob(job);
    }
    
    private void ClearCompletedJobs()
    {
        var manager = BatchJobManager.Instance;
        var completed = manager.jobs.Where(j => 
            j.status == BatchJob.JobStatus.Completed || 
            j.status == BatchJob.JobStatus.Cancelled).ToList();
        
        foreach (var job in completed)
        {
            manager.MoveToHistory(job);
        }
    }
}

