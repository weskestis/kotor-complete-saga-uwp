from pathlib import Path
from xml.etree import ElementTree
import re

root=Path(__file__).resolve().parents[1]
proj=root/'xbox/KotorSagaUWP/KotorSagaUWP.vcxproj'
manifest=root/'xbox/KotorSagaUWP/Package.appxmanifest'
P=ElementTree.parse(proj)
M=ElementTree.parse(manifest)
ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003',
    'a':'http://schemas.microsoft.com/appx/manifest/foundation/windows10',
    'u':'http://schemas.microsoft.com/appx/manifest/uap/windows10'}
assert P.find('.//m:WindowsTargetPlatformVersion',ns).text=='10.0.22621.0'
assert P.find('.//m:CompileAsWinRT',ns).text=='true'
assert {el.attrib['Include'] for el in P.findall('.//m:ClCompile',ns) if 'Include' in el.attrib} >= {
    'App.cpp','GraphicsHost.cpp',r'..\..\src\GraphicsMenu.cpp',r'..\..\src\FramePacer.cpp'}
assert M.find('.//a:TargetDeviceFamily',ns).attrib['Name']=='Windows.Universal'
assert M.find('.//a:Identity',ns).attrib['Version']=='0.1.3.0'
for img in P.findall('.//m:Image',ns):
    assert (proj.parent/img.attrib['Include'].replace('\\','/')).is_file(), img.attrib
for f in ['App.cpp','GraphicsHost.cpp','GraphicsHost.hpp','Package.appxmanifest']:
    assert (proj.parent/f).is_file(),f
assert not list(root.rglob('*.pfx')), 'Never ship private signing keys'
print('XML and required project assets consistent')

workflow=root/'.github/workflows/uwp-x64.yml'
script=root/'ci/build-uwp.ps1'
assert workflow.is_file() and script.is_file()
assert 'New-SelfSignedCertificate' in script.read_text()
assert 'msbuild.binlog' in script.read_text()
assert (root/'ci/verify-package.ps1').is_file()
assert 'AppxSignature.p7x' in (root/'ci/verify-package.ps1').read_text()
assert 'FATAL render' in (proj.parent/'App.cpp').read_text()
assert 'diagnostics.log' in (proj.parent/'App.cpp').read_text()
assert 'Remove-Item -Path $storePath' in script.read_text()
assert 'signing cert' in script.read_text().lower()
assert 'CP1D' in (proj.parent/'GraphicsHost.cpp').read_text()
assert 'CreateTexture2D' in (proj.parent/'GraphicsHost.cpp').read_text()
assert 'sceneBitmap_' in (proj.parent/'GraphicsHost.cpp').read_text()
assert '4K/120 SELECTION DOES NOT GUARANTEE' in (proj.parent/'GraphicsHost.cpp').read_text()
print('CP1D: offscreen scene, diagnostics, and Windows CI packaging validated structurally')
