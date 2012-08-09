@echo off

rem **************************************************************************
rem Uninstall AMMO RMCastPlugin as a Windows service
rem **************************************************************************

echo Uninstalling AMMO RMCastPlugin ...

rem Detect plugin's install directory
set PLUGIN_BASE=%PROGRAMFILES(X86)%\ammo-gateway\bin
if not defined programfiles(x86) (
  set PLUGIN_BASE=%PROGRAMFILES%\ammo-gateway\bin
)

pushd "%PLUGIN_BASE%"

JavaService -uninstall "AMMO RMCast Plugin"

popd

