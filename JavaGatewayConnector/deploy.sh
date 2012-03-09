#!/bin/bash

VERSION=1.3.9
REPO_URI=http://fiddler.aterrasys.com:8081/nexus/content/repositories/vanderbilt/
GW_API=gatewaypluginapi

mvn deploy:deploy-file \
  -DgroupId=edu.vu.isis.ammo \
  -DartifactId=${GW_API} \
  -Dversion=${VERSION} \
  -Dpackaging=jar \
  -Dfile=dist/lib/${GW_API}-${VERSION}.jar \
  -Dsources=dist/lib/${GW_API}-${VERSION}-sources.jar \
  -Djavadoc=dist/lib/${GW_API}-${VERSION}-javadoc.jar \
  -DrepositoryId=nexus-vanderbilt \
  -DgeneratePom=true \
  -Durl=${REPO_URI}

