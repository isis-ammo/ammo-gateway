#!/bin/bash

echo "Checking for Java ..."

JAVA_CMD="java"
if [ -n "${JAVA_HOME+x}" ]; then
  JAVA_CMD="${JAVA_HOME}/bin/java"
fi

$JAVA_CMD -version
if [ $? -ne 0 ]; then
  echo '------------------- ERROR -------------------'
  echo 'MCastPlugin.bar requires Java'
  exit 1
fi

echo 'Java found'

if [ -n "${GATEWAY_ROOT+x}" ]; then
  echo 'Found GATEWAY_ROOT ... running MCastPlugin from there ...'

  pushd "$GATEWAY_ROOT/MCastPlugin"

  $JAVA_CMD -Djava.net.preferIPv4Stack=true -cp \
      libs/gatewaypluginapi.jar:libs/slf4j-api-1.6.4.jar:libs/slf4j-simple-1.6.4.jar:libs/json-20090211.jar:libs/jgroups-gw.jar:libs/protobuf-java-2.4.1.jar:dist/lib/mcastplugin.jar \
      edu.vu.isis.ammo.mcastplugin.McastPlugin

  popd

else
  echo 'Running MCastPlugin from system directory ...'

  pushd /usr/share/java

  $JAVA_CMD -Djava.net.preferIPv4Stack=true -cp \
    gatewaypluginapi.jar:slf4j-api-1.6.4.jar:slf4j-simple-1.6.4.jar:json-20090211.jar:jgroups-gw.jar:protobuf-java-2.4.1.jar:mcastplugin.jar \
    edu.vu.isis.ammo.mcastplugin.McastPlugin

  popd

fi

