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
  echo MCastPlugin.bat requires Java
  exit /b 1
)

echo Java found

if defined GATEWAY_ROOT (
  echo Found GATEWAY_ROOT ... running MCastPlugin from there ...

  pushd "%GATEWAY_ROOT%\MCastPlugin"

  %JAVA_CMD% -cp ^
      libs\gatewaypluginapi.jar;libs\slf4j-api-1.6.4.jar;libs\logback-core-1.0.11.jar;libs\logback-classic-1.0.11.jar;libs\logback-access-1.0.11.jar;libs\json-20090211.jar;libs\protobuf-java-2.4.1.jar;dist\lib\mcastplugin.jar;config/win32 ^
      edu.vu.isis.ammo.mcastplugin.MCastPlugin

  popd

  exit /b 0
) 


echo Running MCastPlugin from system directory ...

set PLUGIN_BASE=%PROGRAMFILES(X86)%\ammo-gateway\bin
if not defined programfiles(x86) (
  set PLUGIN_BASE=%PROGRAMFILES%\ammo-gateway\bin
)

pushd "%PLUGIN_BASE%"

%JAVA_CMD% -cp ^
    gatewaypluginapi.jar;slf4j-api-1.6.4.jar;logback-core-1.0.11.jar;logback-classic-1.0.11.jar;logback-access-1.0.11.jar;json-20090211.jar;protobuf-java-2.4.1.jar;mcastplugin.jar ^
    edu.vu.isis.ammo.mcastplugin.MCastPlugin

popd

exit /b 0

