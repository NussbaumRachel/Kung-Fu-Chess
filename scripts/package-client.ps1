$ErrorActionPreference = "Stop"

$projectRoot = Split-Path `
    -Parent `
    $PSScriptRoot

$buildDirectory = Join-Path `
    $projectRoot `
    "build-clean"

$executablePath = Join-Path `
    $buildDirectory `
    "Release\kungfuchess_client.exe"

$packageDirectory = Join-Path `
    $projectRoot `
    "dist\KungFuChessClient"

$archivePath = Join-Path `
    $projectRoot `
    "dist\KungFuChessClient-Windows-x64.zip"

Write-Host "Building Release client..."

cmake --build $buildDirectory `
    --config Release `
    --target kungfuchess_client

if ($LASTEXITCODE -ne 0)
{
    throw "Client build failed"
}

if (-not (Test-Path $executablePath))
{
    throw "Client executable was not created: $executablePath"
}

Write-Host "Creating package directory..."

Remove-Item `
    $packageDirectory `
    -Recurse `
    -Force `
    -ErrorAction SilentlyContinue

New-Item `
    -ItemType Directory `
    -Path $packageDirectory `
    -Force |
    Out-Null

Copy-Item `
    $executablePath `
    $packageDirectory

$openCvDll = Get-ChildItem `
    (Join-Path $projectRoot "OpenCV_451") `
    -Recurse `
    -Filter "opencv_world451.dll" |
    Select-Object -First 1

if (-not $openCvDll)
{
    throw "opencv_world451.dll was not found"
}

Copy-Item `
    $openCvDll.FullName `
    $packageDirectory

$assetsSource = Join-Path `
    $projectRoot `
    "demo\assets"
    
$assetsDestination = Join-Path `
    $packageDirectory `
    "assets"

if (-not (Test-Path $assetsSource))
{
    throw "Assets directory was not found: $assetsSource"
}

Copy-Item `
    $assetsSource `
    $assetsDestination `
    -Recurse `
    -Force

$runScriptSource = Join-Path `
    $projectRoot `
    "scripts\run-client.ps1"

if (Test-Path $runScriptSource)
{
    Copy-Item `
        $runScriptSource `
        $packageDirectory
}

Write-Host "Creating ZIP archive..."

Remove-Item `
    $archivePath `
    -Force `
    -ErrorAction SilentlyContinue

Compress-Archive `
    -Path "$packageDirectory\*" `
    -DestinationPath $archivePath `
    -Force

Write-Host "Package created:"
Write-Host $archivePath