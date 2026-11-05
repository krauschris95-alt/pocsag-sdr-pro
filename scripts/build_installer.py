"""Generate an NSIS per-user installer whose uninstaller removes only packaged files."""
import pathlib,sys,subprocess
root=pathlib.Path(__file__).resolve().parent.parent
package=root/'dist/POCSAG-SDR-Pro'
version=sys.argv[2] if len(sys.argv)>2 else '0.1.8'
compiler=pathlib.Path(sys.argv[1]).resolve()
script=root/'dist/installer.nsi'
def quote(text):
    return str(text).replace('$','$$').replace('"','$\\"')
files=sorted(p for p in package.rglob('*') if p.is_file())
directories=sorted({parent.relative_to(package) for p in files for parent in p.parents if parent!=package and parent.is_relative_to(package)},key=lambda p:len(p.parts),reverse=True)
lines=[
'Unicode true',
'!include "MUI2.nsh"',
'!include "x64.nsh"',
'Name "POCSAG SDR Pro"',
f'OutFile "{quote(root / ("dist/POCSAG-SDR-Pro-"+version+"-windows-x64-setup.exe"))}"',
'InstallDir "$LOCALAPPDATA\\Programs\\POCSAG SDR Pro"',
'RequestExecutionLevel user',
'SetCompressor /SOLID lzma',
f'VIProductVersion "{version}.0"',
'VIAddVersionKey "ProductName" "POCSAG SDR Pro"',
'VIAddVersionKey "FileDescription" "POCSAG SDR Pro Installer"',
'VIAddVersionKey "LegalCopyright" "POCSAG SDR Pro contributors - GPL-3.0-or-later"',
f'VIAddVersionKey "FileVersion" "{version}"',
'!insertmacro MUI_PAGE_WELCOME',
f'!insertmacro MUI_PAGE_LICENSE "{quote(root / "LICENSE")}"',
'!insertmacro MUI_PAGE_DIRECTORY',
'!insertmacro MUI_PAGE_INSTFILES',
'!insertmacro MUI_PAGE_FINISH',
'!insertmacro MUI_UNPAGE_CONFIRM',
'!insertmacro MUI_UNPAGE_INSTFILES',
'!insertmacro MUI_LANGUAGE "French"',
'Function .onInit',
' ${IfNot} ${RunningX64}',
'  MessageBox MB_ICONSTOP "Windows x64 requis."',
'  Abort',
' ${EndIf}',
'FunctionEnd',
'Section "Application"',
' SetShellVarContext current',
f' SetOutPath "$INSTDIR"',
f' File /r "{quote(package)}\\*"',
' WriteUninstaller "$INSTDIR\\Uninstall.exe"',
' CreateDirectory "$SMPROGRAMS\\POCSAG SDR Pro"',
' CreateShortcut "$SMPROGRAMS\\POCSAG SDR Pro\\POCSAG SDR Pro.lnk" "$INSTDIR\\POCSAG-SDR-Pro.exe"',
' WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\POCSAG-SDR-Pro" "DisplayName" "POCSAG SDR Pro"',
f' WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\POCSAG-SDR-Pro" "DisplayVersion" "{version}"',
' WriteRegStr HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\POCSAG-SDR-Pro" "UninstallString" \'$\\"$INSTDIR\\Uninstall.exe$\\"\'',
'SectionEnd',
'Section "Uninstall"',
' SetShellVarContext current',
]
for file in files:
    lines.append(f' Delete "$INSTDIR\\{quote(file.relative_to(package))}"')
for directory in directories:
    lines.append(f' RMDir "$INSTDIR\\{quote(directory)}"')
lines += [
' Delete "$INSTDIR\\Uninstall.exe"',
' RMDir "$INSTDIR"',
' Delete "$SMPROGRAMS\\POCSAG SDR Pro\\POCSAG SDR Pro.lnk"',
' RMDir "$SMPROGRAMS\\POCSAG SDR Pro"',
' DeleteRegKey HKCU "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\POCSAG-SDR-Pro"',
'SectionEnd',
]
script.write_text('\n'.join(lines),encoding='utf-8-sig')
subprocess.run([str(compiler),'/V2',str(script)],check=True)
