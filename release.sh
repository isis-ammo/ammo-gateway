#!/bin/bash

VERSION=$(date --rfc-3339=date)
PLATFORM=$(uname --kernel-release)
TARBALL_NAME="gateway-${VERSION}-${PLATFORM}"

function upload_release() {
   chromium-browser https://ammo.isis.vanderbilt.edu/redmine/projects/code/files/new
}

function build_tarball() {
   mkdir tarball
   tar cvzf "./tarball/${TARBALL_NAME}.tgz" ./bin ./lib ./README
}

pushd ./build
export RELEASE_DIR=$(dirs +0)

select action in 'EXIT' 'ALL' 'upload' 'tarball'; do
  case ${action} in 
    'EXIT') break;;
    'ALL') 
       build_tarball  
       upload_release
      ;;  
    'upload') 
       upload_release 
      ;;  
    'tarball')  
       build_tarball 
      ;;  
  esac
done
popd
