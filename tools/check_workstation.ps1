param(
    [string]$UnrealRoot = "",
    [int]$MinimumFreeGB = 80,
    [string]$DiskDrive = "C",
    [switch]$SkipGpu
)

$ErrorActionPreference = "SilentlyContinue"
$checks = [System.Collections.Generic.List[object]]::new()
$script:blockers = 0

function Add-Check {
    param(
        [string]$Name,
        [bool]$Passed,
        [string]$Detail,
        [bool]$Blocking = $true
    )
    $checks.Add([pscustomobject]@{
        Check = $Name
        Status = if ($Passed) { "PASS" } else { "BLOCK" }
        Detail = $Detail
    })
    if ($Blocking -and -not $Passed) { $script:blockers++ }
}

function Find-UnrealRoot {
    param([string]$RequestedRoot)
    if ($RequestedRoot -and (Test-Path $RequestedRoot)) {
        return (Resolve-Path $RequestedRoot).Path
    }
    foreach ($epicRoot in @('C:\Program Files\Epic Games','C:\Program Files (x86)\Epic Games')) {
        if (-not (Test-Path $epicRoot)) { continue }
        $candidate = Get-ChildItem $epicRoot -Directory |
            Where-Object { $_.Name -like 'UE_*' } |
            Sort-Object Name -Descending |
            Select-Object -First 1
        if ($candidate) { return $candidate.FullName }
    }
    return $null
}

function Invoke-CppSmoke {
    param([string]$VsPath)
    if (-not $VsPath) {
        return [pscustomobject]@{ Passed = $false; Detail = 'Visual Studio path unavailable' }
    }
    $devCmd = Join-Path $VsPath 'Common7\Tools\VsDevCmd.bat'
    if (-not (Test-Path $devCmd)) {
        return [pscustomobject]@{ Passed = $false; Detail = 'VsDevCmd.bat missing' }
    }
    $tmp = Join-Path $env:TEMP ("OpenDriveCity-CppSmoke-{0}" -f $PID)
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    New-Item -ItemType Directory -Force -Path $tmp | Out-Null
    $source = '#include <windows.h>' + [Environment]::NewLine +
        'int main(){ SYSTEM_INFO s{}; GetSystemInfo(&s); return s.dwNumberOfProcessors > 0 ? 0 : 1; }'
    Set-Content -Path (Join-Path $tmp 'smoke.cpp') -Value $source -Encoding Ascii
    $cmdFile = Join-Path $tmp 'run-smoke.cmd'
    $cmdLines = @(
        '@echo off',
        ('call "{0}" -arch=x64 -host_arch=x64 >nul' -f $devCmd),
        'if errorlevel 1 exit /b %errorlevel%',
        ('cd /d "{0}"' -f $tmp),
        'cl /nologo /EHsc smoke.cpp /Fe:smoke.exe >build.log 2>&1',
        'if errorlevel 1 exit /b %errorlevel%',
        'smoke.exe',
        'exit /b %errorlevel%'
    )
    Set-Content -Path $cmdFile -Value $cmdLines -Encoding Ascii
    & $env:ComSpec /d /c $cmdFile | Out-Null
    $exitCode = $LASTEXITCODE
    $detail = 'VsDevCmd.bat -> cl -> link -> smoke.exe PASS'
    if ($exitCode -ne 0) {
        $log = Join-Path $tmp 'build.log'
        $tail = if (Test-Path $log) { (Get-Content $log -Tail 5) -join ' | ' } else { 'no build log' }
        $detail = "C++ compile smoke failed exit=$exitCode; $tail"
    }
    Remove-Item $tmp -Recurse -Force -ErrorAction SilentlyContinue
    return [pscustomobject]@{ Passed = ($exitCode -eq 0); Detail = $detail }
}

$vswhere = 'C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = $null
if (Test-Path $vswhere) {
    $vsPath = (& $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath | Select-Object -First 1)
}
Add-Check 'Visual Studio C++ Build Tools' ([bool]$vsPath) `
    $(if ($vsPath) { $vsPath } else { 'Visual Studio C++ tools not found' })
$msbuild = if ($vsPath) { Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe' } else { $null }
Add-Check 'MSBuild' ([bool]($msbuild -and (Test-Path $msbuild))) `
    $(if ($msbuild -and (Test-Path $msbuild)) { $msbuild } else { 'MSBuild missing' })
$cl = $null
if ($vsPath) {
    $cl = Get-ChildItem (Join-Path $vsPath 'VC\Tools\MSVC') -Filter cl.exe -Recurse -File |
        Where-Object { $_.FullName -match 'HostX64\\x64\\cl.exe$' } |
        Sort-Object FullName -Descending |
        Select-Object -First 1
}
Add-Check 'MSVC cl.exe' ([bool]$cl) $(if ($cl) { $cl.FullName } else { 'cl.exe missing' })
$smoke = Invoke-CppSmoke $vsPath
Add-Check 'C++ compile smoke' ([bool]$smoke.Passed) $smoke.Detail

$rc = Get-ChildItem 'C:\Program Files (x86)\Windows Kits\10\bin' -Filter rc.exe -Recurse -File |
    Where-Object { $_.FullName -match '\\x64\\rc.exe$' } |
    Sort-Object FullName -Descending |
    Select-Object -First 1
Add-Check 'Windows SDK rc.exe' ([bool]$rc) $(if ($rc) { $rc.FullName } else { 'rc.exe missing' })
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
Add-Check 'cmake' ([bool]$cmake) $(if ($cmake) { $cmake.Source } else { 'cmake missing' })
Add-Check 'ninja' ([bool]$ninja) $(if ($ninja) { $ninja.Source } else { 'ninja missing' })
$launcherCandidates = @(
    'C:\Program Files (x86)\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe',
    'C:\Program Files\Epic Games\Launcher\Portal\Binaries\Win64\EpicGamesLauncher.exe'
)
$launcher = $launcherCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
Add-Check 'Epic Games Launcher' ([bool]$launcher) `
    $(if ($launcher) { $launcher } else { 'Epic Games Launcher not installed' }) $false

$engineRoot = Find-UnrealRoot $UnrealRoot
Add-Check 'Unreal Engine' ([bool]$engineRoot) `
    $(if ($engineRoot) { $engineRoot } else { 'Unreal Engine installation not found' })
$buildBat = if ($engineRoot) { Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat' } else { $null }
Add-Check 'Unreal Build.bat' ([bool]($buildBat -and (Test-Path $buildBat))) `
    $(if ($buildBat -and (Test-Path $buildBat)) { $buildBat } else { 'Build.bat missing' })

$drive = Get-PSDrive -Name $DiskDrive
$FreeGB = if ($drive) { [math]::Round($drive.Free / 1GB, 1) } else { 0 }
Add-Check 'Disk FreeGB' ($FreeGB -ge $MinimumFreeGB) `
    ("Drive={0}; FreeGB={1}; required>={2}" -f $DiskDrive, $FreeGB, $MinimumFreeGB)

if ($SkipGpu) {
    Add-Check 'GPU' $true 'Skipped by -SkipGpu; GPU-runtime-unverified' $false
} else {
    $gpus = Get-CimInstance Win32_VideoController
    $usableGpu = $gpus | Where-Object {
        $_.Name -notmatch 'VMware|Remote Display|Microsoft Basic|Microsoft Remote'
    } | Select-Object -First 1
    Add-Check 'GPU' ([bool]$usableGpu) `
        $(if ($usableGpu) { $usableGpu.Name } else { 'No discrete GPU detected; GPU-runtime-unverified' })
}

$checks | Format-Table -AutoSize
if ($script:blockers -gt 0) {
    Write-Host ("WORKSTATION_PREFLIGHT=BLOCKED blockers={0}" -f $script:blockers)
    exit 2
}

Write-Host 'WORKSTATION_PREFLIGHT=PASS'
exit 0
