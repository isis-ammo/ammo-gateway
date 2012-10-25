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

!ifndef QTDIR
  !error "You must set QTDIR."
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
!include "x64.nsh"

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
!define MUI_ICON "Manager\icons\manager.ico"

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
;Macros

; Find JavaHome in the registry and return it in $0. If not found,
; set $0 to an empty string. Note: Only checks currently set registry.
; If you need to check the 64 or 32 bit registries specifically, you must use 
; SetRegView before calling this macro. 
!macro FindJavaHome
  !define UniqueID_FindJavaHome ${__LINE__}

  ; Place CurrentVersion in $0 or jump to no_java if not available.
  ReadRegStr $0 HKLM "SOFTWARE\JavaSoft\Java Runtime Environment" "CurrentVersion"
  StrCmp $0 "" no_java_${UniqueID_FindJavaHome}

  ; Place JavaHome in $0 using CurrentVersion or just to no_java if not available.
  ReadRegStr $0 HKLM "SOFTWARE\JavaSoft\Java Runtime Environment\$0" "JavaHome"
  StrCmp $0 "" no_java_${UniqueID_FindJavaHome}

  ; Return JavaHome
  Goto end_${UniqueID_FindJavaHome}

  no_java_${UniqueID_FindJavaHome}:

  ; Return an empty string
  StrCpy $0 ""
  Goto end_${UniqueID_FindJavaHome}

  end_${UniqueID_FindJavaHome}:

  !undef UniqueID_FindJavaHome
!macroend

; Find jvm.dll and return it to $0. If not found, set $0 to an empty string.
!macro FindJvmDll
  !define UniqueID_FindJvmDll ${__LINE__}

  ; Try 64 bit registry first.
  try_64_${UniqueID_FindJvmDll}:
  SetRegView 64

  ; Get 64 bit JavaHome
  !insertmacro FindJavaHome

  ; Look in 64 bit server directory
  try_server_64_${UniqueID_FindJvmDll}:
  IfFileExists "$0\bin\server\jvm.dll" 0 try_client_64_${UniqueID_FindJvmDll}
  StrCpy $0 "$0\bin\server\jvm.dll"
  Goto success_${UniqueID_FindJvmDll}

  ; Look in 64 bit client directory
  try_client_64_${UniqueID_FindJvmDll}:
  IfFileExists "$0\bin\client\jvm.dll" 0 try_32_${UniqueID_FindJvmDll}
  StrCpy $0 "$0\bin\client\jvm.dll"
  Goto success_${UniqueID_FindJvmDll}

  ; Try 32 bit registry next.
  try_32_${UniqueID_FindJvmDll}:
  SetRegView 64

  ; Get 32 bit JavaHome
  !insertmacro FindJavaHome

  ; Look in 32 bit server directory
  try_server_32_${UniqueID_FindJvmDll}:
  IfFileExists "$0\bin\server\jvm.dll" 0 try_client_32_${UniqueID_FindJvmDll}
  StrCpy $0 "$0\bin\server\jvm.dll"
  Goto success_${UniqueID_FindJvmDll}

  ; Look in 32 bit client directory
  try_client_32_${UniqueID_FindJvmDll}:
  IfFileExists "$0\bin\client\jvm.dll" 0 failed_${UniqueID_FindJvmDll}
  StrCpy $0 "$0\bin\client\jvm.dll"
  Goto success_${UniqueID_FindJvmDll}

  failed_${UniqueID_FindJvmDll}:
  ; Set $0 to empty string indicating failure
  StrCpy $0 ""
  Goto end_${UniqueID_FindJvmDll}

  success_${UniqueID_FindJvmDll}:
  ; nop - $0 already contains location of jvm.dll
  Goto end_${UniqueID_FindJvmDll}

  end_${UniqueID_FindJvmDll}:

  ; Restore registry view to 32-bit
  SetRegView 32

  !undef UniqueID_FindJvmDll
!macroend

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

  SetShellVarContext all
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\AndroidPluginConfig.json

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

${MementoSection} "Serial Gateway Plugin (required)" SecSerPlug

  SetDetailsPrint textonly
  DetailPrint "Installing Serial Plugin ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SimpleSC::StopService "SerialGatewayPlugin" "1" "30"
  SimpleSC::RemoveService "SerialGatewayPlugin"

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File build\bin\SerialGatewayPlugin.exe
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\SerialPluginConfig.json

  SimpleSC::InstallService "SerialGatewayPlugin" "AMMO Serial Gateway Plugin" 16 2 "$INSTDIR\bin\SerialGatewayPlugin.exe" "GatewayCore" "" ""

${MementoSectionEnd}

${MementoSection} "Java Gateway Connector (required)" SecJavaConn

  SetDetailsPrint textonly
  DetailPrint "Installing Java Connector ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File JavaGatewayConnector\dist\lib\gatewaypluginapi.jar

${MementoSectionEnd}

${MementoSection} "MCast Gateway Plugin (required)" SecMCastPlug

  SetDetailsPrint textonly
  DetailPrint "MCast Java Plugin ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  ${If} ${RunningX64}  ; also used by RMCastPlugin
    File /oname=JavaService.exe JavaService\JavaService64.exe
  ${Else}
    File JavaService\JavaService.exe
  ${EndIf}
  File MCastPlugin\mcastplugin.bat
  File MCastPlugin\dist\lib\mcastplugin.jar
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\MCastPluginConfig.json

  SetShellVarContext all
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\MCastPluginConfig.json

  !insertmacro FindJvmDll
  ${If} $0 == ""
    Goto mcast_no_java
  ${EndIf}

  ExpandEnvStrings $1 "%COMSPEC%"
  ExecWait '"$1" /C IF 1==1 "$INSTDIR\bin\JavaService.exe" \
               -install \
               "AMMO MCast Plugin" \
               "$0" \
               -Djava.net.preferIPv4Stack=true \
               "-Djava.class.path=$INSTDIR\bin\gatewaypluginapi.jar;$INSTDIR\bin\slf4j-api-1.6.4.jar;$INSTDIR\bin\slf4j-simple-1.6.4.jar;$INSTDIR\bin\json-20090211.jar;$INSTDIR\bin\jgroups-gw.jar;$INSTDIR\bin\protobuf-java-2.3.0.jar;$INSTDIR\bin\mcastplugin.jar" \
               -start edu.vu.isis.ammo.mcastplugin.MCastPlugin \
               -depends "GatewayCore" \
               -description "AMMO MCast Plugin" ' $0
  ${If} $0 != "0"
    MessageBox MB_OK|MB_ICONEXCLAMATION "MCastPlugin failed to install. Error Code: $0"
  ${EndIf}
  Goto mcast_the_end

  mcast_no_java:
  MessageBox MB_OK|MB_ICONEXCLAMATION "MCastPlugin failed to install. ERROR: Java was not found"
  Goto mcast_the_end

  mcast_the_end:

${MementoSectionEnd}

${MementoSection} "RMCast Gateway Plugin (required)" SecRMCastPlug

  SetDetailsPrint textonly
  DetailPrint "RMCast Java Plugin ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO
  ;SetOutPath $INSTDIR
  ;RMDir /r $SMPROGRAMS\ammo-gateway

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File RMCastPlugin\rmcastplugin.bat
  File RMCastPlugin\dist\lib\rmcastplugin.jar
  File RMCastPlugin\libs\jgroups-gw.jar
  ; jars from here down are also prereqs for mcastplugin
  File RMCastPlugin\libs\json-20090211.jar
  File RMCastPlugin\libs\protobuf-java-2.3.0.jar
  File RMCastPlugin\libs\slf4j-api-1.6.4.jar
  File RMCastPlugin\libs\slf4j-simple-1.6.4.jar
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\RMCastPluginConfig.json

  SetShellVarContext all
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\RMCastPluginConfig.json

  SetOutPath $APPDATA\ammo-gateway\jgroups
  File RMCastPlugin\jgroups\udp.xml

  !insertmacro FindJvmDll
  ${If} $0 == ""
    Goto rmcast_no_java
  ${EndIf}

  ExpandEnvStrings $1 "%COMSPEC%"
  ExecWait '"$1" /C IF 1==1 "$INSTDIR\bin\JavaService.exe" \
               -install \
               "AMMO RMCast Plugin" \
               "$0" \
               -Djava.net.preferIPv4Stack=true \
               "-Djava.class.path=$INSTDIR\bin\gatewaypluginapi.jar;$INSTDIR\bin\slf4j-api-1.6.4.jar;$INSTDIR\bin\slf4j-simple-1.6.4.jar;$INSTDIR\bin\json-20090211.jar;$INSTDIR\bin\jgroups-gw.jar;$INSTDIR\bin\protobuf-java-2.3.0.jar;$INSTDIR\bin\rmcastplugin.jar" \
               -start edu.vu.isis.ammo.rmcastplugin.RMCastPlugin \
               -depends "GatewayCore" \
               -description "AMMO RMCast Plugin" ' $0
  ${If} $0 != "0"
    MessageBox MB_OK|MB_ICONEXCLAMATION "RMCastPlugin failed to install. Error Code: $0"
  ${EndIf}
  Goto rmcast_the_end

  rmcast_no_java:
  MessageBox MB_OK|MB_ICONEXCLAMATION "RMCastPlugin failed to install. ERROR: Java was not found"
  Goto rmcast_the_end

  rmcast_the_end:

${MementoSectionEnd}

${MementoSection} "Manager (required)" SecManager

  SetDetailsPrint textonly
  DetailPrint "Installing Manager ..."
  SetDetailsPrint listonly

  SectionIn 1 2 3 RO

  SetOutPath $INSTDIR\bin
  SetOverwrite on
  File "Manager\release\Manager.exe"
  File "${QTDIR}\bin\QtCore4.dll"
  File "${QTDIR}\bin\QtGui4.dll"

  SetShellVarContext all
  SetOutPath $APPDATA\ammo-gateway
  File build\etc\win32\ManagerConfig.json

  CreateDirectory "$SMPROGRAMS\AMMO Gateway"
  CreateShortcut "$SMPROGRAMS\AMMO Gateway\Manager.lnk" "$INSTDIR\bin\Manager.exe"

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

  ; Make sure to use the 32-bit registry
  SetRegView 32

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
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ammo-gateway" "DisplayIcon" "$INSTDIR\bin\Manager.exe,0"
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
  SimpleSC::StartService "SerialGatewayPlugin" "" "30"
  SimpleSC::StartService "AMMO MCast Plugin" "" "30"
  SimpleSC::StartService "AMMO RMCast Plugin" "" "30"

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
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSerPlug} "The Serial Plugin Service for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecJavaConn} "The Java Connector for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMCastPlug} "The MCast Plugin Service for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecRMCastPlug} "The RMCast Plugin Service for AMMO Gateway"
  !insertmacro MUI_DESCRIPTION_TEXT ${SecManager} "The Gateway Manager"
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

  ; Make sure to use the 32-bit registry
  SetRegView 32

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

  ; Make sure to use the 32-bit registry
  SetRegView 32

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
  SimpleSC::StopService "SerialGatewayPlugin" "1" "30"
  SimpleSC::StopService "AMMO MCast Plugin" "1" "30"
  SimpleSC::StopService "AMMO RMCast Plugin" "1" "30"
  SimpleSC::RemoveService "GatewayCore"
  SimpleSC::RemoveService "AndroidGatewayPlugin"
  SimpleSC::RemoveService "DataStoreGatewayPlugin"
  ;SimpleSC::RemoveService "LdapGatewayPlugin"
  SimpleSC::RemoveService "SerialGatewayPlugin"
  ExecWait '"$INSTDIR\bin\JavaService.exe" -uninstall "AMMO MCast Plugin"'
  ExecWait '"$INSTDIR\bin\JavaService.exe" -uninstall "AMMO RMCast Plugin"'

  ; Gateway Core
  Delete $INSTDIR\bin\GatewayCore.exe
  Delete $INSTDIR\bin\gatewayconnector.dll

  ; Android Gateway Plugin
  Delete $INSTDIR\bin\AndroidGatewayPlugin.exe

  ; LDAP Gateway Plugin
  ;Delete $INSTDIR\bin\LdapGatewayPlugin.exe

  ; Serial Gateway Plugin
  Delete $INSTDIR\bin\SerialGatewayPlugin.exe

  ; Data Store Gateway Plugin
  Delete $INSTDIR\bin\DataStoreGatewayPlugin.exe

  ; Java Gateway Connector
  Delete $INSTDIR\bin\gatewaypluginapi.jar

  ; MCast Plugin
  Delete $INSTDIR\bin\JavaService.exe
  Delete $INSTDIR\bin\mcastplugin.bat
  Delete $INSTDIR\bin\mcastplugin.jar

  ; RMCast Plugin
  Delete $INSTDIR\bin\rmcastplugin.bat
  Delete $INSTDIR\bin\rmcastplugin.jar
  Delete $INSTDIR\bin\jgroups-gw.jar
  Delete $INSTDIR\bin\json-20090211.jar
  Delete $INSTDIR\bin\protobuf-java-2.3.0.jar
  Delete $INSTDIR\bin\slf4j-api-1.6.4.jar
  Delete $INSTDIR\bin\slf4j-simple-1.6.4.jar

  ; Manager
  Delete "Manager\release\Manager.exe"
  Delete "$INSTDIR\bin\QtCore4.dll"
  Delete "$INSTDIR\bin\QtGui4.dll"
  Delete "$APPDATA\ammo-gateway\ManagerConfig.json"
  Delete "$SMPROGRAMS\AMMO Gateway\Manager.lnk"

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
  RMDir "$SMPROGRAMS\AMMO Gateway"

  SetDetailsPrint both

SectionEnd

