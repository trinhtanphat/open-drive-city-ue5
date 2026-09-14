param(
    [string]$UnrealRoot = "",
    [string]$ProjectPath = "",
    [int]$TimeoutSeconds = 180
)

$ErrorActionPreference = "Stop"

function Find-UnrealRoot {
    param([string]$RequestedRoot)
    if ($RequestedRoot) {
        if (-not (Test-Path -LiteralPath $RequestedRoot)) { return $null }
        return (Resolve-Path -LiteralPath $RequestedRoot).Path
    }
    foreach ($epicRoot in @('C:\Program Files\Epic Games','C:\Program Files (x86)\Epic Games')) {
        if (-not (Test-Path -LiteralPath $epicRoot)) { continue }
        $preferred = Join-Path $epicRoot 'UE_5.6'
        if (Test-Path -LiteralPath $preferred) { return (Resolve-Path -LiteralPath $preferred).Path }
        $candidate = Get-ChildItem -LiteralPath $epicRoot -Directory |
            Where-Object { $_.Name -like 'UE_*' } |
            Sort-Object Name -Descending |
            Select-Object -First 1
        if ($candidate) { return $candidate.FullName }
    }
    return $null
}

$engineRoot = Find-UnrealRoot $UnrealRoot
if (-not $engineRoot) {
    Write-Output "UNREAL_SMOKE=BLOCKED reason=Unreal Engine installation not found"
    exit 2
}

$editorCmd = Join-Path $engineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $editorCmd)) {
    Write-Output "UNREAL_SMOKE=BLOCKED reason=UnrealEditor-Cmd.exe missing path=$editorCmd"
    exit 2
}

if (-not $ProjectPath) {
    $ProjectPath = Join-Path $PSScriptRoot '..\OpenDriveCity.uproject'
}
if (-not (Test-Path -LiteralPath $ProjectPath)) {
    Write-Output "UNREAL_SMOKE=BLOCKED reason=Project file missing path=$ProjectPath"
    exit 2
}
$project = (Resolve-Path -LiteralPath $ProjectPath).Path

Write-Output "UNREAL_SMOKE_ENGINE=$engineRoot"
Write-Output "UNREAL_SMOKE_PROJECT=$project"
Write-Output "UNREAL_SMOKE_TIMEOUT_SECONDS=$TimeoutSeconds"

$smokeArgs = @(
    "`"$project`"",
    "-NullRHI",
    "-Unattended",
    "-NoSplash",
    "-NoSound",
    "-ExecCmds=Quit",
    "-stdout",
    "-FullStdOutLogOutput"
)

$process = Start-Process -FilePath $editorCmd -ArgumentList $smokeArgs -PassThru
if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
    Stop-Process -Id $process.Id -Force -ErrorAction SilentlyContinue
    Write-Output "UNREAL_SMOKE=FAIL reason=timeout seconds=$TimeoutSeconds"
    exit 3
}

$code = $process.ExitCode
if ($code -ne 0) {
    Write-Output "UNREAL_SMOKE=FAIL exit=$code"
    exit $code
}

Write-Output "UNREAL_SMOKE=PASS mode=NullRHI"
exit 0
