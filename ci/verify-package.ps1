# Structural verification of generated UWP APPX/MSIX and its public signer.
# Not a substitute for installation and runtime verification on Xbox Series X.
# SPDX-License-Identifier: GPL-3.0-or-later
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$PackageDir,
    [switch]$Signed
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$packages = @(Get-ChildItem -Path $PackageDir -Recurse -File | Where-Object { $_.Extension -in @('.appx','.msix') })
if ($packages.Count -eq 0) { throw "No APPX/MSIX package under $PackageDir" }
if ($packages.Count -gt 1) { Write-Warning 'More than one APPX found. Checking all.' }
$expectedPublisher = 'CN=KotorSagaDev'
if ($Signed) {
    $pub = Join-Path $PackageDir 'KotorSagaDev-PUBLIC.cer'
    if (-not (Test-Path $pub)) { throw 'Public signer certificate is missing.' }
    $certificate = New-Object System.Security.Cryptography.X509Certificates.X509Certificate2($pub)
    if ($certificate.Subject -ne $expectedPublisher) { throw 'Certificate does not match the app identity publisher.' }
    if ($certificate.HasPrivateKey) { throw 'Refusing to ship a private key.' }
}
foreach ($package in $packages) {
    $archive = [System.IO.Compression.ZipFile]::OpenRead($package.FullName)
    try {
        $entry = $archive.Entries | Where-Object { $_.FullName -eq 'AppxManifest.xml' } | Select-Object -First 1
        if (-not $entry) { throw "APPX missing AppxManifest.xml: $($package.Name)" }
        $reader = New-Object System.IO.StreamReader($entry.Open())
        try { [xml]$manifest = $reader.ReadToEnd() } finally { $reader.Dispose() }
        $ns = New-Object System.Xml.XmlNamespaceManager($manifest.NameTable)
        $ns.AddNamespace('a','http://schemas.microsoft.com/appx/manifest/foundation/windows10')
        $identity = $manifest.SelectSingleNode('//a:Identity',$ns)
        if (-not $identity) { throw 'Package identity missing from packed manifest.' }
        if ($identity.GetAttribute('Name') -ne 'KotorSaga.CP1D.PlatformProbe') { throw "Unexpected identity $($identity.GetAttribute('Name'))" }
        if ($identity.GetAttribute('Publisher') -ne $expectedPublisher) { throw 'Packaged publisher mismatch.' }
        if ($identity.GetAttribute('Version') -ne '0.1.3.0') { throw 'Incorrect package version.' }
        $cpu = $identity.GetAttribute('ProcessorArchitecture')
        if ($cpu -and $cpu -ne 'x64') { throw "Unexpected package architecture $cpu" }
        if ($Signed -and -not ($archive.Entries | Where-Object { $_.FullName -eq 'AppxSignature.p7x' })) { throw 'Signed package has no signature entry.' }
        $sizeKiB = [math]::Round($package.Length/1KB,1)
        Write-Host "Verified $($package.Name) ($sizeKiB KiB), Version $($identity.GetAttribute('Version')), CPU $cpu"
    } finally { $archive.Dispose() }
}
'Native gameplay: NOT INCLUDED. Xbox runtime: NOT VERIFIED.' | Set-Content -Path (Join-Path $PackageDir 'PACKAGE-READ-ME.txt')
