param(
    [string]$UnrealRoot = "",
    [string]$ProjectPath = "",
    [string]$Target = "OpenDriveCityEditor",
    [string]$Platform = "Win64",
    [string]$Configuration = "Development"
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
    Write-Output "BUILD_UNREAL=BLOCKED reason=Unreal Engine installation not found"
    exit 2
}

$buildBat = Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path -LiteralPath $buildBat)) {
    Write-Output "BUILD_UNREAL=BLOCKED reason=Build.bat missing path=$buildBat"
    exit 2
}

if (-not $ProjectPath) {
    $ProjectPath = Join-Path $PSScriptRoot '..\OpenDriveCity.uproject'
}
if (-not (Test-Path -LiteralPath $ProjectPath)) {
    Write-Output "BUILD_UNREAL=BLOCKED reason=Project file missing path=$ProjectPath"
    exit 2
}
$project = (Resolve-Path -LiteralPath $ProjectPath).Path

Write-Output "BUILD_UNREAL_ENGINE=$engineRoot"
Write-Output "BUILD_UNREAL_PROJECT=$project"
Write-Output "BUILD_UNREAL_TARGET=$Target platform=$Platform configuration=$Configuration"

$buildArgs = @(
    $Target,
    $Platform,
    $Configuration,
    "-Project=`"$project`"",
    "-WaitMutex",
    "-FromMsBuild"
)

& $buildBat @buildArgs
$code = $LASTEXITCODE
if ($code -ne 0) {
    Write-Output "BUILD_UNREAL=FAIL exit=$code"
    exit $code
}

Write-Output "BUILD_UNREAL=PASS target=$Target platform=$Platform configuration=$Configuration"
exit 0
