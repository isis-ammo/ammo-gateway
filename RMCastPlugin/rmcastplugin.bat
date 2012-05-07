@echo off

rem Checking if Java is installed
echo Checking for Java ...

set JAVA_CMD="java.exe"
if defined JAVA_HOME (
  set JAVA_CMD="%JAVA_HOME%\bin\java.exe"
)

%JAVA_CMD% -version
if %ERRORLEVEL% NEQ 0 (
  echo ------------------- ERROR -------------------
  echo RMCastPlugin.bat requires Java
  exit /b 1
)

echo Java found

if defined GATEWAY_ROOT (
  echo Found GATEWAY_ROOT ... running MCastPlugin from there ...

  pushd "%GATEWAY_ROOT%\Gateway\RMCastPlugin"

  %JAVA_CMD% -Djava.net.preferIPv4Stack=true -cp ^
      libs\gatewaypluginapi.jar;libs\slf4j-api-1.6.4.jar;libs\slf4j-simple-1.6.4.jar;libs\json-20090211.jar;libs\jgroups-gw.jar;libs\protobuf-java-2.3.0.jar;dist\lib\rmcastplugin.jar ^
      edu.vu.isis.ammo.rmcastplugin.RMCastPlugin

  popd

  exit /b 0
) 


echo Running RMCastPlugin from system directory ...

set PLUGIN_BASE=%PROGRAMFILES%\ammo-gateway\bin
if defined PROGRAMFILES(X86) (
  set PLUGIN_BASE=%PROGRAMFILES(X86)%\ammo-gateway\bin
)

pushd "%PLUGIN_BASE%"

%JAVA_CMD% -Djava.net.preferIPv4Stack=true -cp ^
    gatewaypluginapi.jar;slf4j-api-1.6.4.jar;slf4j-simple-1.6.4.jar;json-20090211.jar;jgroups-gw.jar;protobuf-java-2.3.0.jar;rmcastplugin.jar ^
    edu.vu.isis.ammo.rmcastplugin.RMCastPlugin

popd

exit /b 0

