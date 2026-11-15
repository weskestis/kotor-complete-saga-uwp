# Build the independent Xbox UWP CP1D platform probe and create a development-signed APPX.
# SPDX-License-Identifier: GPL-3.0-or-later
# SECURITY: no private key / PFX ever leaves this short-lived runner.
[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Release',
    [ValidateSet('x64')][string]$Platform = 'x64',
    [string]$PackageDir = '',
    [switch]$Unsigned
)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($PackageDir)) { $PackageDir = Join-Path $root 'appx-output' }
$PackageDir = [System.IO.Path]::GetFullPath($PackageDir)
New-Item -Type Directory -Path $PackageDir -Force | Out-Null
$solution = Join-Path $root 'xbox\KotorSagaUWP.sln'
$transcriptStarted = $false
$cert = $null
try {
    Start-Transcript -Path (Join-Path $PackageDir 'build-transcript.txt') -Force | Out-Null
    $transcriptStarted = $true
    if (-not (Test-Path $solution)) { throw "Solution missing: $solution" }

    $msbuildCmd = Get-Command msbuild.exe -ErrorAction SilentlyContinue
    $msbuild = if ($msbuildCmd) { $msbuildCmd.Source } else { $null }
    if (-not $msbuild) {
        $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
        if (-not (Test-Path $vswhere)) { throw 'MSBuild / Visual Studio 2022 not found.' }
        $msbuild = & $vswhere -latest -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1
    }
    if (-not $msbuild -or -not (Test-Path $msbuild)) { throw 'MSBuild executable missing.' }

    $sdkVersion = '10.0.22621.0'
    $sdkPath = Join-Path "${env:ProgramFiles(x86)}\Windows Kits\10\Include" $sdkVersion
    if (-not (Test-Path $sdkPath)) {
        $available = Get-ChildItem (Join-Path "${env:ProgramFiles(x86)}\Windows Kits\10\Include") -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Name
        throw "Required Windows SDK $sdkVersion missing. Available: $($available -join ', ')"
    }
    Write-Host "CP1D build: $Configuration / $Platform; Windows SDK $sdkVersion; MSBuild $msbuild"

    $sign = 'false'
    if (-not $Unsigned) {
        $cert = New-SelfSignedCertificate -Subject 'CN=KotorSagaDev' -Type CodeSigningCert `
            -CertStoreLocation 'Cert:\CurrentUser\My' -KeyAlgorithm RSA -KeyLength 2048 `
            -HashAlgorithm SHA256 -NotAfter (Get-Date).AddDays(30)
        if (-not $cert) { throw 'Failed to create development signing certificate.' }
        Export-Certificate -Cert $cert -FilePath (Join-Path $PackageDir 'KotorSagaDev-PUBLIC.cer') | Out-Null
        $sign = 'true'
        Write-Host "Ephemeral signer created: $($cert.Thumbprint) (public certificate only in artifact)"
    }

    $safePackageDir = $PackageDir.TrimEnd([char[]]@('\','/')).Replace('\','/') + '/'
    $args = @(
        $solution, '/restore', '/m:2', '/v:normal',
        "-p:Configuration=$Configuration", "-p:Platform=$Platform",
        '-p:AppxBundle=Never', '-p:GenerateAppxPackageOnBuild=true',
        '-p:UapAppxPackageBuildMode=SideloadOnly',
        "-p:AppxPackageDir=$safePackageDir",
        "-p:AppxPackageSigningEnabled=$sign",
        "/bl:$PackageDir\msbuild.binlog"
    )
    if ($cert) { $args += "-p:PackageCertificateThumbprint=$($cert.Thumbprint)" }
    & $msbuild @args
    if ($LASTEXITCODE -ne 0) { throw "MSBuild failed with exit code $LASTEXITCODE. Inspect msbuild.binlog." }

    & (Join-Path $PSScriptRoot 'verify-package.ps1') -PackageDir $PackageDir -Signed:(-not $Unsigned)
    if ($LASTEXITCODE -and $LASTEXITCODE -ne 0) { throw 'Package verifier failed.' }
    Write-Host 'CP1D Windows build and package verification completed.'
}
finally {
    if ($cert) {
        $storePath = "Cert:\CurrentUser\My\$($cert.Thumbprint)"
        if (Test-Path $storePath) { Remove-Item -Path $storePath -Force }
    }
    if ($transcriptStarted) { Stop-Transcript | Out-Null }
}
