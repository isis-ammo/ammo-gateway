;NSIS Setup Script
;--------------------------------

!ifndef ACE_ROOT
  !error "You must set ACE_ROOT."
!endif

!ifndef PROTOBUF_ROOT
  !error "You must set PROTOBUF_ROOT."
!endif

!ifndef VC_ROOT
  !define VC_ROOT "C:\Program Files\Microsoft Visual Studio 10.0\VC"
!endif

!ifndef VERSION
  !error "You must set VERSION."
!endif

;--------------------------------
;Configuration

!ifdef OUTFILE
  OutFile "${OUTFILE}"
!else
  OutFile ammo-gateway-${VERSION}.exe
!endif

SetCompressor /SOLID lzma

InstType "Full"


InstallDir $PROGRAMFILES\ammo-gateway
InstallDirRegKey HKLM Software\ammo-gateway ""

RequestExecutionLevel admin

;--------------------------------
;Header Files

!include "MUI2.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "Memento.nsh"
!include "WordFunc.nsh"

;--------------------------------
;Functions

!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD

  !insertmacro VersionCompare

!endif

;--------------------------------
;Definitions

!define SHCNE_ASSOCCHANGED 0x8000000
!define SHCNF_IDLIST 0

;--------------------------------
;Configuration

;Names
Name "AMMO Gateway"
Caption "AMMO Gateway ${VERSION} Setup"

;Memento Settings
!define MEMENTO_REGISTRY_ROOT HKLM
!define MEMENTO_REGISTRY_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway"

;Icon setting
;!define MUI_ICON path_to_icon.ico

;Interface Settings
!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\nsis.bmp"

!define MUI_COMPONENTSPAGE_SMALLDESC

;Pages
!define MUI_WELCOMEPAGE_TITLE "Welcome to the AMMO Gateway ${VERSION} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of AMMO Gateway ${VERSION}.$\r$\n$\r$\n$_CLICK"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
Page custom PageReinstall PageLeaveReinstall
!endif
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the AMMO site for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://ammo.isis.vanderbilt.edu"

;!define MUI_FINISHPAGE_RUN "$INSTDIR\NSIS.exe"
;!define MUI_FINISHPAGE_NOREBOOTSUPPORT

;!define MUI_FINISHPAGE_SHOWREADME
;!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show release notes"
;!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReleaseNotes

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

${MementoSection} "Gateway Core (required)" SecCore

  SetDetailsPrint textonly
  DetailPrint "Installing Gateway Core ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SimpleSC::StopService "GatewayCore" "1" "30"
  SimpleSC::RemoveService "GatewayCore"

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File build\bin\GatewayCore.exe
  File build\lib\gatewayconnector.dll

  SetShellVarContext all
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\GatewayConfig.json
  File build\etc\win32\LoggingConfig.json

  SimpleSC::InstallService "GatewayCore" "AMMO Gateway Core" 16 2 "$INSTDIR\bin\GatewayCore.exe" "" "" ""

${MementoSectionEnd}

${MementoSection} "Android Gateway Plugin (required)" SecAndPlug

  SetDetailsPrint textonly
  DetailPrint "Installing Android Plugin ..."
  SetDetailsPrint listonly

  SimpleSC::StopService "AndroidGatewayPlugin" "1" "30"
  SimpleSC::RemoveService "AndroidGatewayPlugin"

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File build\bin\AndroidGatewayPlugin.exe

  SimpleSC::InstallService "AndroidGatewayPlugin" "AMMO Android Gateway Plugin" 16 2 "$INSTDIR\bin\AndroidGatewayPlugin.exe" "GatewayCore" "" ""

${MementoSectionEnd}

;${MementoSection} "LDAP Gateway Plugin (required)" SecLdapPlug
;
;  SetDetailsPrint textonly
;  DetailPrint "Installing LDAP Plugin ..."
;  SetDetailsPrint listonly
;
;  SimpleSC::StopService "LdapGatewayPlugin" "1" "30"
;  SimpleSC::RemoveService "LdapGatewayPlugin"
;
;  SectionIn 1 2 3 RO
;  ;SetOutPath $INSTDIR
;  ;RMDir /r $SMPROGRAMS\ammo-gateway
;
;  SetOutPath $INSTDIR\bin
;  SetOverwrite on
;  File build\bin\LdapGatewayPlugin.exe
;  SetOutPath $APPDATA\ammo-gateway
;  File build\etc\win32\LdapPluginConfig.json
;
;  SimpleSC::InstallService "LdapGatewayPlugin" "AMMO LDAP Gateway Plugin" 16 2 "$INSTDIR\bin\LdapGatewayPlugin.exe" "GatewayCore" "" ""
;
;${MementoSectionEnd}

${MementoSection} "Data Store Gateway Plugin (required)" SecDatPlug

  SetDetailsPrint textonly
  DetailPrint "Installing Data Store Plugin ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SimpleSC::StopService "DataStoreGatewayPlugin" "1" "30"
  SimpleSC::RemoveService "DataStoreGatewayPlugin"

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File build\bin\DataStoreGatewayPlugin.exe
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\DataStorePluginConfig.json

  SimpleSC::InstallService "DataStoreGatewayPlugin" "AMMO Data Store Gateway Plugin" 16 2 "$INSTDIR\bin\DataStoreGatewayPlugin.exe" "GatewayCore" "" ""

${MementoSectionEnd}

${MementoSection} "VC Redist (required)" SecVcredist

  SetDetailsPrint textonly
  DetailPrint "Installing VC redist ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File "${VC_ROOT}\redist\x86\Microsoft.VC100.CRT\msvcp100.dll"
  File "${VC_ROOT}\redist\x86\Microsoft.VC100.CRT\msvcr100.dll"

${MementoSectionEnd}

${MementoSection} "ACE (required)" SecAce

  SetDetailsPrint textonly
  DetailPrint "Installing ACE ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File ${ACE_ROOT}\lib\ACE.dll
  File ${ACE_ROOT}\lib\ACEd.dll

${MementoSectionEnd}

${MementoSection} "Sqlite (required)" SecSqlite

  SetDetailsPrint textonly
  DetailPrint "Installing Sqlite ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File ${PROTOBUF_ROOT}\lib\sqlite3.dll

${MementoSectionEnd}

${MementoSection} "JSON (required)" SecJson

  SetDetailsPrint textonly
  DetailPrint "Installing JSON ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File build\lib\JSON.dll
  ;File build\lib\JSONd.dll

${MementoSectionEnd}

${MementoSectionDone}

Section -post

  ; When Modern UI is installed:
  ; * Always install the English language file
  ; * Always install default icons / bitmaps

  !insertmacro SectionFlagIsSet ${SecInterfacesModernUI} ${SF_SELECTED} mui nomui

    mui:

    SetDetailsPrint textonly
    DetailPrint "Configuring Modern UI..."
    SetDetailsPrint listonly

    !insertmacro SectionFlagIsSet ${SecLangFiles} ${SF_SELECTED} langfiles nolangfiles

      nolangfiles:

      ;SetOutPath "$INSTDIR\Contrib\Language files"
      ;File "..\Contrib\Language files\English.nlf"
      ;SetOutPath "$INSTDIR\Contrib\Language files"
      ;File "..\Contrib\Language files\English.nsh"

    langfiles:

    !insertmacro SectionFlagIsSet ${SecGraphics} ${SF_SELECTED} graphics nographics

      nographics:

      ;SetOutPath $INSTDIR\Contrib\Graphics
      ;SetOutPath $INSTDIR\Contrib\Graphics\Checks
      ;File "..\Contrib\Graphics\Checks\modern.bmp"
      ;SetOutPath $INSTDIR\Contrib\Graphics\Icons
      ;File "..\Contrib\Graphics\Icons\modern-install.ico"
      ;File "..\Contrib\Graphics\Icons\modern-uninstall.ico"
      ;SetOutPath $INSTDIR\Contrib\Graphics\Header
      ;File "..\Contrib\Graphics\Header\nsis.bmp"
      ;SetOutPath $INSTDIR\Contrib\Graphics\Wizard
      ;File "..\Contrib\Graphics\Wizard\win.bmp"

    graphics:

  nomui:

  SetDetailsPrint textonly
  DetailPrint "Creating Registry Keys..."
  SetDetailsPrint listonly

  SetOutPath $INSTDIR

  WriteRegStr HKLM "Software\ammo-gateway" "" $INSTDIR
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  WriteRegDword HKLM "Software\ammo-gateway" "VersionMajor" "${VER_MAJOR}"
  WriteRegDword HKLM "Software\ammo-gateway" "VersionMinor" "${VER_MINOR}"
  WriteRegDword HKLM "Software\ammo-gateway" "VersionRevision" "${VER_REVISION}"
  WriteRegDword HKLM "Software\ammo-gateway" "VersionBuild" "${VER_BUILD}"
!endif

  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "UninstallString" '"$INSTDIR\uninst-ammo-gateway.exe"'
  WriteRegExpandStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "DisplayName" "AMMO Gateway"
  ;WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "DisplayIcon" "$INSTDIR\bin\GatewayCore.exe,0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "DisplayVersion" "${VERSION}"
!ifdef VER_MAJOR & VER_MINOR & VER_REVISION & VER_BUILD
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "VersionMajor" "${VER_MAJOR}"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "VersionMinor" "${VER_MINOR}.${VER_REVISION}"
!endif
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "URLInfoAbout" "http://nsis.sourceforge.net/"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "HelpLink" "http://ammo.isis.vanderbilt.edu"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "NoRepair" "1"

  WriteUninstaller $INSTDIR\uninst-ammo-gateway.exe

  ; Start Windows services
  SimpleSC::StartService "GatewayCore" "" "30"
  SimpleSC::StartService "AndroidGatewayPlugin" "" "30"
  ;SimpleSC::StartService "LdapGatewayPlugin" "" "30"
  SimpleSC::StartService "DataStoreGatewayPlugin" "" "30"

  ${MementoSectionSave}

  SetDetailsPrint both

SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The Gateway's Core Service"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAndPlug} "The Android Plugin Service for AMMO Gateway"
  ;!insertmacro MUI_DESCRIPTION_TEXT ${SecLdapPlug} "The LDAP Plugin Service for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDatPlug} "The Data Store Plugin Service for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecVcredist} "The VC redist dependency"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAce} "The ACE networking dependency"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSqlite} "The SQLite database dependency"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecJson} "The JSON serialization dependency"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Installer Functions

Function .onInit

  ; Check if we actually have admin rights and bail if not.
  UserInfo::GetAccountType
  pop $0
  ${If} $0 != "admin"
    MessageBox mb_iconstop "You must have Administrator rights to install AMMO Gateway"
    SetErrorLevel 740  ; ERROR_ELEVATION_REQUIRED
    Quit
  ${EndIf}

  ReadRegStr $R0 HKLM \
  "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" \
  "UninstallString"
  StrCmp $R0 "" uninst_done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
  "AMMO Gateway is already installed. $\n$\nClick `OK` to remove the \
  previous version or `Cancel` to cancel this upgrade." \
  IDOK uninst
  Abort
 
;Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller uninst_done
    ;You can either use Delete /REBOOTOK in the uninstaller or add some code
    ;here to remove the uninstaller. Use a registry key to check
    ;whether the user has chosen to uninstall. If you are using an uninstaller
    ;components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
uninst_done:
 
  ${MementoSectionRestore}

FunctionEnd

;--------------------------------
;Uninstaller Section

Section Uninstall

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway"
  DeleteRegKey HKLM "Software\ammo-gateway"

  SetDetailsPrint textonly
  DetailPrint "Deleting Files..."
  SetDetailsPrint listonly

  ; Windows services
  SimpleSC::StopService "GatewayCore" "1" "30"
  SimpleSC::StopService "AndroidGatewayPlugin" "1" "30"
  SimpleSC::StopService "DataStoreGatewayPlugin" "1" "30"
  ;SimpleSC::StopService "LdapGatewayPlugin" "1" "30"
  ;SimpleSC::StopService "SerialGatewayPlugin" "1" "30"
  SimpleSC::RemoveService "GatewayCore"
  SimpleSC::RemoveService "AndroidGatewayPlugin"
  SimpleSC::RemoveService "DataStoreGatewayPlugin"
  ;SimpleSC::RemoveService "LdapGatewayPlugin"
  ;SimpleSC::RemoveService "SerialGatewayPlugin"

  ; Gateway Core
  Delete $INSTDIR\bin\GatewayCore.exe
  Delete $INSTDIR\bin\gatewayconnector.dll

  ; Android Gateway Plugin
  Delete $INSTDIR\bin\AndroidGatewayPlugin.exe

  ; LDAP Gateway Plugin
  ;Delete $INSTDIR\bin\LdapGatewayPlugin.exe

  ; Data Store Gateway Plugin
  Delete $INSTDIR\bin\DataStoreGatewayPlugin.exe

  ; ACE
  Delete $INSTDIR\bin\ACE.dll
  Delete $INSTDIR\bin\ACEd.dll
  
  ; ACE
  Delete $INSTDIR\bin\sqlite.dll

  ; JSON
  Delete $INSTDIR\bin\JSON.dll
  Delete $INSTDIR\bin\JSONd.dll

  ; uninstaller
  Delete $INSTDIR\uninst-ammo-gateway.exe

  ; directory cleanup
  RMDir /r $INSTDIR\bin
  RMDir $INSTDIR

  SetDetailsPrint both

SectionEnd

