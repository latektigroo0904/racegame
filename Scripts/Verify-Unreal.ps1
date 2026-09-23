param(
    [string]$UERoot = $env:UE_ROOT,
    [switch]$SkipGenerateProjectFiles,
    [switch]$SkipBuild
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$ProjectPath = Join-Path $RepoRoot "TorqueAtlas.uproject"

if (-not (Test-Path $ProjectPath)) {
    throw "TorqueAtlas.uproject not found at $ProjectPath"
}

if ([string]::IsNullOrWhiteSpace($UERoot)) {
    $Candidates = @(
        "C:\Program Files\Epic Games\UE_5.8",
        "D:\Program Files\Epic Games\UE_5.8",
        "D:\Epic Games\UE_5.8"
    )

    $UERoot = $Candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if ([string]::IsNullOrWhiteSpace($UERoot) -or -not (Test-Path $UERoot)) {
    throw "UE 5.8 root not found. Set UE_ROOT or pass -UERoot."
}

$UERoot = (Resolve-Path $UERoot).Path
$BuildVersionPath = Join-Path $UERoot "Engine\Build\Build.version"

if (-not (Test-Path $BuildVersionPath)) {
    throw "Unreal Build.version not found under $UERoot"
}

$EngineVersion = Get-Content $BuildVersionPath -Raw | ConvertFrom-Json

if ($EngineVersion.MajorVersion -ne 5 -or $EngineVersion.MinorVersion -ne 8) {
    throw "Expected Unreal Engine 5.8, found $($EngineVersion.MajorVersion).$($EngineVersion.MinorVersion)."
}

$Stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$VerificationRoot = Join-Path $RepoRoot "Saved\Verification\$Stamp"
$ReportRoot = Join-Path $VerificationRoot "AutomationReport"
$BuildLog = Join-Path $VerificationRoot "build.log"
$AutomationLog = Join-Path $VerificationRoot "automation.log"
$MetadataPath = Join-Path $VerificationRoot "metadata.json"

New-Item -ItemType Directory -Force -Path $ReportRoot | Out-Null

$GitSha = "unknown"
try {
    $GitSha = (& git -C $RepoRoot rev-parse HEAD 2>$null).Trim()
} catch {
    $GitSha = "unknown"
}

$Metadata = [ordered]@{
    started_utc = (Get-Date).ToUniversalTime().ToString("o")
    repo_root = $RepoRoot
    git_sha = $GitSha
    unreal_root = $UERoot
    unreal_version = "$($EngineVersion.MajorVersion).$($EngineVersion.MinorVersion).$($EngineVersion.PatchVersion)"
    project = $ProjectPath
    target = "TorqueAtlasEditor"
    platform = "Win64"
    configuration = "Development"
    automation_filter = "TorqueAtlas."
    status = "running"
    build_exit_code = $null
    automation_exit_code = $null
}

function Write-Metadata {
    $Metadata | ConvertTo-Json -Depth 6 | Set-Content -Encoding UTF8 $MetadataPath
}

function Invoke-NativeLogged {
    param(
        [Parameter(Mandatory=$true)][string]$FilePath,
        [Parameter(Mandatory=$true)][string[]]$Arguments,
        [Parameter(Mandatory=$true)][string]$LogPath
    )

    Write-Host ">> $FilePath $($Arguments -join ' ')"
    & $FilePath @Arguments 2>&1 | Tee-Object -FilePath $LogPath
    return $LASTEXITCODE
}

Write-Metadata

try {
    if (-not $SkipGenerateProjectFiles) {
        $Generate = Join-Path $UERoot "Engine\Build\BatchFiles\GenerateProjectFiles.bat"

        if (Test-Path $Generate) {
            $GenerateArgs = @("-project=$ProjectPath", "-game", "-engine")
            $GenerateLog = Join-Path $VerificationRoot "generate-project-files.log"
            $GenerateExit = Invoke-NativeLogged -FilePath $Generate -Arguments $GenerateArgs -LogPath $GenerateLog

            if ($GenerateExit -ne 0) {
                throw "GenerateProjectFiles failed with exit code $GenerateExit."
            }
        } else {
            Write-Warning "GenerateProjectFiles.bat not found; continuing directly to UBT build."
        }
    }

    if (-not $SkipBuild) {
        $Build = Join-Path $UERoot "Engine\Build\BatchFiles\Build.bat"

        if (-not (Test-Path $Build)) {
            throw "Build.bat not found at $Build"
        }

        $BuildArgs = @(
            "TorqueAtlasEditor",
            "Win64",
            "Development",
            "-Project=$ProjectPath",
            "-WaitMutex",
            "-NoHotReload"
        )

        $BuildExit = Invoke-NativeLogged -FilePath $Build -Arguments $BuildArgs -LogPath $BuildLog
        $Metadata.build_exit_code = $BuildExit
        Write-Metadata

        if ($BuildExit -ne 0) {
            throw "UnrealBuildTool build failed with exit code $BuildExit."
        }
    }

    $EditorCmd = Join-Path $UERoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"

    if (-not (Test-Path $EditorCmd)) {
        throw "UnrealEditor-Cmd.exe not found at $EditorCmd"
    }

    $AutomationArgs = @(
        $ProjectPath,
        "-Unattended",
        "-NullRHI",
        "-NoSplash",
        "-NoSound",
        "-stdout",
        "-FullStdOutLogOutput",
        "-ExecCmds=Automation RunTests TorqueAtlas.",
        "-TestExit=Automation Test Queue Empty",
        "-ReportExportPath=$ReportRoot",
        "-abslog=$AutomationLog"
    )

    $AutomationStdout = Join-Path $VerificationRoot "automation-stdout.log"
    $AutomationExit = Invoke-NativeLogged -FilePath $EditorCmd -Arguments $AutomationArgs -LogPath $AutomationStdout
    $Metadata.automation_exit_code = $AutomationExit

    if ($AutomationExit -ne 0) {
        throw "Unreal Automation failed with exit code $AutomationExit."
    }

    $Metadata.status = "passed"
    $Metadata.finished_utc = (Get-Date).ToUniversalTime().ToString("o")
    Write-Metadata

    Write-Host ""
    Write-Host "Torque Atlas UE verification completed successfully."
    Write-Host "Artifacts: $VerificationRoot"
    exit 0
} catch {
    $Metadata.status = "failed"
    $Metadata.failure = $_.Exception.Message
    $Metadata.finished_utc = (Get-Date).ToUniversalTime().ToString("o")
    Write-Metadata

    Write-Error $_
    Write-Host "Artifacts: $VerificationRoot"
    exit 1
}
