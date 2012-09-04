@echo off

rem **************************************************************************
rem Install AMMO RMCastPlugin as a Windows service
rem
rem Installation may be corrupted during Java upgrades because the location
rem of jvm.dll moves. In this case, run the corresponding uninstallation
rem script followed by this script again to correct the situation.
rem
rem Error Codes:
rem    1 - Could not find CurVer of Java in Windows registry
rem    2 - Could not find location of JVM DLL in Windows registry
rem    3 - Could not find JVM DLL at location as registered or alternative
rem **************************************************************************

echo Installing AMMO RMCastPlugin ...

rem Detect Java's current version for registry crawling
FOR /F "skip=2 tokens=2*" %%A IN ('REG QUERY "HKLM\Software\JavaSoft\Java Runtime Environment" /v CurrentVersion') DO set CurVer=%%B
if not defined CurVer (
  echo ERROR: Java not found - Failed to find CurVer
  echo Skipping AMMO RMCastPlugin installation
  echo You must install Java before running rmcastplugin-install.bat
  exit /b 1
)
echo Found Java version: %CurVer%

rem Detect Java's jvm.dll
FOR /F "skip=2 tokens=2*" %%A IN ('REG QUERY "HKLM\Software\JavaSoft\Java Runtime Environment\%CurVer%" /v RuntimeLib') DO set JVM_DLL=%%B
if not defined JVM_DLL (
  echo ERROR: Java not found - Failed to find RuntimeLib
  echo Skipping AMMO RMCastPlugin installation
  echo You must install Java before running rmcastplugin-install.bat
  exit /b 2
)
if not exist "%JVM_DLL%" (
  set JVM_DLL=%JVM_DLL:client=server%
)
if not exist "%JVM_DLL%" (
  echo ERROR: Java not found - JVM does not appear at the location declared in the registry nor the alternate location.
  echo Skipping AMMO RMCastPlugin installation
  echo You must install Java before running rmcastplugin-install.bat
  exit /b 3
)
echo Found jvm.dll at %JVM_DLL%

rem Detect plugin's install directory
set PLUGIN_BASE=%PROGRAMFILES(X86)%\ammo-gateway\bin
if not defined programfiles(x86) (
  set PLUGIN_BASE=%PROGRAMFILES%\ammo-gateway\bin
)

pushd "%PLUGIN_BASE%"

JavaService -install "AMMO RMCast Plugin" "%JVM_DLL%" ^
    -Djava.net.preferIPv4Stack=true ^
    "-Djava.class.path=%PLUGIN_BASE%\gatewaypluginapi.jar;%PLUGIN_BASE%\slf4j-api-1.6.4.jar;%PLUGIN_BASE%\slf4j-simple-1.6.4.jar;%PLUGIN_BASE%\json-20090211.jar;%PLUGIN_BASE%\jgroups-gw.jar;%PLUGIN_BASE%\protobuf-java-2.3.0.jar;%PLUGIN_BASE%\rmcastplugin.jar" ^
    -start edu.vu.isis.ammo.rmcastplugin.RMCastPlugin ^
    -depends "GatewayCore" ^
    -description "AMMO RMCast Plugin" 

popd

exit /b 0

