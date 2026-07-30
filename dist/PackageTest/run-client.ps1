param(
    [Parameter(Mandatory = $true)]
    [string]$Username,

    [string]$HostName = "127.0.0.1",

    [int]$Port = 8080,

    [ValidateSet("", "create", "join", "leave")]
    [string]$Action = "",

    [string]$RoomId = ""
)

$ErrorActionPreference = "Stop"

$clientPath = Join-Path `
    $PSScriptRoot `
    "kungfuchess_client.exe"

$assetsPath = Join-Path `
    $PSScriptRoot `
    "assets"

if (-not (Test-Path $clientPath))
{
    throw "Client executable was not found: $clientPath"
}

if (-not (Test-Path $assetsPath))
{
    throw "Assets directory was not found: $assetsPath"
}

$clientArguments = @(
    $assetsPath
    $Username
    $HostName
    $Port.ToString()
)

switch ($Action)
{
    ""
    {
        # Login only.
    }

    "create"
    {
        if ([string]::IsNullOrWhiteSpace($RoomId))
        {
            throw "RoomId is required for create"
        }

        $clientArguments += "create"
        $clientArguments += $RoomId
    }

    "join"
    {
        if ([string]::IsNullOrWhiteSpace($RoomId))
        {
            throw "RoomId is required for join"
        }

        $clientArguments += "join"
        $clientArguments += $RoomId
    }

    "leave"
    {
        $clientArguments += "leave"
    }
}

& $clientPath @clientArguments

exit $LASTEXITCODE