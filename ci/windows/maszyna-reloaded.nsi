; Windows installer of the exported game: one file to download instead of a zip, whose exe run
; from inside the archive starts without libmaszyna.64.dll. Built by `make release-windows` in
; ci/docker/windows-installer with:
;   SOURCE_DIR   - the unpacked export (reloaded.exe, libmaszyna.64.dll)
;   BUILD_NUMBER - demo/build_number.txt
;   OUTFILE      - the installer to write
; Installed per user, so it needs no administrator rights.

Unicode true
!include "MUI2.nsh"

!define PRODUCT_NAME "MaSzyna Reloaded"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
; the starts for diagnosing a crash: another renderer, no Python cab screens
; (PythonScreenServer::ARG_NO_PYTHON), Godot's verbose log
!define SHORTCUT_D3D12 "${PRODUCT_NAME} (D3D12)"
!define SHORTCUT_NO_PYTHON "${PRODUCT_NAME} (no Python)"
!define SHORTCUT_VERBOSE "${PRODUCT_NAME} (verbose)"

Name "${PRODUCT_NAME}"
OutFile "${OUTFILE}"
RequestExecutionLevel user
InstallDir "$LOCALAPPDATA\Programs\${PRODUCT_NAME}"
InstallDirRegKey HKCU "${UNINSTALL_KEY}" "InstallLocation"
SetCompressor /SOLID lzma

!define MUI_ABORTWARNING
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_FINISHPAGE_RUN "$INSTDIR\reloaded.exe"

!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "SimpChinese"

LangString SECTION_GAME ${LANG_ENGLISH} "Game"
LangString SECTION_GAME ${LANG_POLISH} "Gra"
LangString SECTION_GAME ${LANG_CZECH} "Hra"
LangString SECTION_GAME ${LANG_HUNGARIAN} "Játék"
LangString SECTION_GAME ${LANG_SIMPCHINESE} "游戏"

LangString SECTION_DESKTOP ${LANG_ENGLISH} "Desktop shortcut"
LangString SECTION_DESKTOP ${LANG_POLISH} "Skrót na pulpicie"
LangString SECTION_DESKTOP ${LANG_CZECH} "Zástupce na ploše"
LangString SECTION_DESKTOP ${LANG_HUNGARIAN} "Asztali parancsikon"
LangString SECTION_DESKTOP ${LANG_SIMPCHINESE} "桌面快捷方式"

Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function un.onInit
    !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Section "!$(SECTION_GAME)" SectionGame
    SectionIn RO
    SetOutPath "$INSTDIR"
    File "${SOURCE_DIR}/reloaded.exe"
    File "${SOURCE_DIR}/libmaszyna.64.dll"
    WriteUninstaller "$INSTDIR\uninstall.exe"

    CreateShortcut "$SMPROGRAMS\${PRODUCT_NAME}.lnk" "$INSTDIR\reloaded.exe"
    CreateShortcut "$SMPROGRAMS\${SHORTCUT_D3D12}.lnk" "$INSTDIR\reloaded.exe" "--rendering-driver d3d12"
    CreateShortcut "$SMPROGRAMS\${SHORTCUT_NO_PYTHON}.lnk" "$INSTDIR\reloaded.exe" "--no-python"
    CreateShortcut "$SMPROGRAMS\${SHORTCUT_VERBOSE}.lnk" "$INSTDIR\reloaded.exe" "--verbose"

    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${BUILD_NUMBER}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\reloaded.exe"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1
SectionEnd

Section "$(SECTION_DESKTOP)" SectionDesktop
    CreateShortcut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\reloaded.exe"
    CreateShortcut "$DESKTOP\${SHORTCUT_D3D12}.lnk" "$INSTDIR\reloaded.exe" "--rendering-driver d3d12"
    CreateShortcut "$DESKTOP\${SHORTCUT_NO_PYTHON}.lnk" "$INSTDIR\reloaded.exe" "--no-python"
    CreateShortcut "$DESKTOP\${SHORTCUT_VERBOSE}.lnk" "$INSTDIR\reloaded.exe" "--verbose"
SectionEnd

; only the installed files are removed: the directory may have been chosen by hand and hold more
Section "Uninstall"
    Delete "$INSTDIR\reloaded.exe"
    Delete "$INSTDIR\reloaded.console.exe"
    Delete "$INSTDIR\libmaszyna.64.dll"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"
    Delete "$SMPROGRAMS\${PRODUCT_NAME}.lnk"
    Delete "$SMPROGRAMS\${SHORTCUT_D3D12}.lnk"
    Delete "$SMPROGRAMS\${SHORTCUT_NO_PYTHON}.lnk"
    Delete "$SMPROGRAMS\${SHORTCUT_VERBOSE}.lnk"
    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
    Delete "$DESKTOP\${SHORTCUT_D3D12}.lnk"
    Delete "$DESKTOP\${SHORTCUT_NO_PYTHON}.lnk"
    Delete "$DESKTOP\${SHORTCUT_VERBOSE}.lnk"
    DeleteRegKey HKCU "${UNINSTALL_KEY}"
SectionEnd
