using System;
using System.IO;
using System.Collections;
using UnityEditor;
using UnityEngine;
using UnityEngine.Networking;
using System.Collections.Generic;

/// <summary>
/// Executable file downloader for Unity Asset Store distribution
/// Automatically downloads RundeeItemFactory.exe from GitHub Releases or CDN
/// </summary>
public class ExecutableDownloader
{
    private static ExecutableDownloader instance;
    private static ExecutableDownloader Instance
    {
        get
        {
            if (instance == null)
            {
                instance = new ExecutableDownloader();
            }
            return instance;
        }
    }
    
    private Dictionary<string, IEnumerator> activeCoroutines = new Dictionary<string, IEnumerator>();
    
    // ============================================
    // 배포 설정: 아래 URL을 실제 배포 URL로 변경하세요
    // ============================================
    
    // GitHub Releases URL 형식:
    // - Latest: https://github.com/USERNAME/REPO/releases/latest/download/RundeeItemFactory.exe
    // - Specific version: https://github.com/USERNAME/REPO/releases/download/v1.0.0/RundeeItemFactory.exe
    private const string GITHUB_RELEASES_URL = "https://github.com/yourusername/RundeeItemFactory/releases/latest/download/RundeeItemFactory.exe";
    
    // Alternative: CDN URL (예: GitHub Pages, Cloudflare, etc.)
    // private const string CDN_URL = "https://your-cdn.com/RundeeItemFactory.exe";
    
    // GitHub Repository 정보 (버전 확인용)
    private const string GITHUB_REPO = "yourusername/RundeeItemFactory"; // "USERNAME/REPO" 형식
    
    // 다운로드할 실행 파일의 버전 (체크섬 검증용, 선택사항)
    private const string EXPECTED_VERSION = "1.0.0"; // 필요시 업데이트
    
    /// <summary>
    /// 다운로드할 실행 파일의 URL
    /// </summary>
    public static string DownloadUrl => GITHUB_RELEASES_URL;
    
    // EditorApplication.update를 사용한 코루틴 에뮬레이션
    private ExecutableDownloader()
    {
    }
    
    private void Update()
    {
        var keys = new List<string>(activeCoroutines.Keys);
        foreach (var key in keys)
        {
            try
            {
                if (!activeCoroutines[key].MoveNext())
                {
                    activeCoroutines.Remove(key);
                }
            }
            catch (Exception e)
            {
                UnityEngine.Debug.LogError($"[ExecutableDownloader] Coroutine error: {e.Message}");
                activeCoroutines.Remove(key);
            }
        }
        
        if (activeCoroutines.Count == 0)
        {
            EditorApplication.update -= Update;
        }
    }
    
    private void StartCoroutine(string key, IEnumerator coroutine)
    {
        activeCoroutines[key] = coroutine;
        if (activeCoroutines.Count == 1)
        {
            EditorApplication.update += Update;
        }
    }
    
    /// <summary>
    /// 실행 파일이 다운로드되어야 할 경로
    /// </summary>
    public static string GetDownloadPath()
    {
        string projectRoot = Application.dataPath.Replace("/Assets", "");
        return Path.Combine(projectRoot, "RundeeItemFactory.exe");
    }
    
    /// <summary>
    /// 실행 파일이 이미 다운로드되어 있는지 확인
    /// </summary>
    public static bool IsExecutableDownloaded()
    {
        string path = GetDownloadPath();
        return File.Exists(path);
    }
    
    /// <summary>
    /// 실행 파일 다운로드 (비동기)
    /// </summary>
    /// <param name="onProgress">진행률 콜백 (0.0 ~ 1.0)</param>
    /// <param name="onComplete">완료 콜백 (성공 여부, 에러 메시지)</param>
    public static void DownloadExecutable(Action<float> onProgress = null, Action<bool, string> onComplete = null)
    {
        string downloadPath = GetDownloadPath();
        string downloadDir = Path.GetDirectoryName(downloadPath);
        
        // 디렉토리 생성
        if (!Directory.Exists(downloadDir))
        {
            Directory.CreateDirectory(downloadDir);
        }
        
        // 기존 파일이 있으면 백업
        if (File.Exists(downloadPath))
        {
            string backupPath = downloadPath + ".backup";
            if (File.Exists(backupPath))
            {
                File.Delete(backupPath);
            }
            File.Move(downloadPath, backupPath);
        }
        
        Instance.StartCoroutine("DownloadExecutable", Instance.DownloadCoroutine(DownloadUrl, downloadPath, onProgress, onComplete));
    }
    
    private IEnumerator DownloadCoroutine(string url, string savePath, Action<float> onProgress, Action<bool, string> onComplete)
    {
        using (UnityWebRequest request = UnityWebRequest.Get(url))
        {
            request.downloadHandler = new DownloadHandlerFile(savePath);
            
            var operation = request.SendWebRequest();
            
            // 진행률 업데이트
            while (!operation.isDone)
            {
                float progress = operation.progress;
                onProgress?.Invoke(progress);
                yield return null;
            }
            
            // 완료 처리
            if (request.result == UnityWebRequest.Result.Success)
            {
                // 파일 다운로드 성공
                AssetDatabase.Refresh();
                onComplete?.Invoke(true, null);
            }
            else
            {
                // 에러 발생
                string error = $"다운로드 실패: {request.error}";
                
                // 백업 파일 복원
                string backupPath = savePath + ".backup";
                if (File.Exists(backupPath))
                {
                    if (File.Exists(savePath))
                    {
                        File.Delete(savePath);
                    }
                    File.Move(backupPath, savePath);
                }
                
                onComplete?.Invoke(false, error);
            }
        }
    }
    
    /// <summary>
    /// 실행 파일 버전 확인 (선택사항)
    /// </summary>
    public static void CheckVersion(Action<string> onVersionReceived = null)
    {
        // GitHub Releases API를 사용하여 최신 버전 확인
        string versionUrl = $"https://api.github.com/repos/{GITHUB_REPO}/releases/latest";
        
        Instance.StartCoroutine("CheckVersion", Instance.CheckVersionCoroutine(versionUrl, onVersionReceived));
    }
    
    private IEnumerator CheckVersionCoroutine(string url, Action<string> onVersionReceived)
    {
        using (UnityWebRequest request = UnityWebRequest.Get(url))
        {
            yield return request.SendWebRequest();
            
            if (request.result == UnityWebRequest.Result.Success)
            {
                try
                {
                    // JSON 파싱 (간단한 버전)
                    string json = request.downloadHandler.text;
                    // "tag_name": "v1.0.0" 형식에서 버전 추출
                    int tagIndex = json.IndexOf("\"tag_name\"");
                    if (tagIndex >= 0)
                    {
                        int startIndex = json.IndexOf("\"", tagIndex + 10) + 1;
                        int endIndex = json.IndexOf("\"", startIndex);
                        if (startIndex > 0 && endIndex > startIndex)
                        {
                            string version = json.Substring(startIndex, endIndex - startIndex);
                            onVersionReceived?.Invoke(version);
                        }
                    }
                }
                catch (Exception e)
                {
                    UnityEngine.Debug.LogWarning($"[ExecutableDownloader] 버전 확인 실패: {e.Message}");
                }
            }
        }
    }
}

