#!/bin/bash

set -e

BUILDS_URL="https://ammo.isis.vanderbilt.edu/builds/Gateway/"
GREP_PATTERN="\"ammo-gateway.*.deb\""
SED_PATTERN="s/.*\"\(ammo-gateway.*deb\)\".*/\1/"

MAX_VMAJOR=0
MAX_VMINOR=0
MAX_VMICRO=0
MAX_VBUILD=0
MAX_DEB=""

function setNewestDeb
{
    MAX_VMAJOR=$!
    MAX_VMINOR=$2
    MAX_VMICRO=$3
    MAX_VBUILD=$4
    MAX_DEB=$5
}

# Create a list of each deb from the builds URL and iterate across them.
# Check for the newest version and record it.
# When done checking them all, install the newest version.
for deb in `curl $BUILDS_URL 2> /dev/null | grep $GREP_PATTERN | sed $SED_PATTERN`
do
    VMAJOR=`echo $deb | sed -n "s/ammo-gateway_\([0-9]\+\)\.[0-9]\+\.[0-9]\+.*/\1/p"`
    VMINOR=`echo $deb | sed -n "s/ammo-gateway_[0-9]\+\.\([0-9]\+\)\.[0-9]\+.*/\1/p"`
    VMICRO=`echo $deb | sed -n "s/ammo-gateway_[0-9]\+\.[0-9]\+\.\([0-9]\+\).*/\1/p"`
    VBUILD=`echo $deb | sed -n "s/ammo-gateway_[0-9]\+\.[0-9]\+\.[0-9]\+-\([0-9]\+\)-.*/\1/p"`
    if [ -z "$VBUILD" ]
    then
        VBUILD=0
    fi

    if (( VMAJOR > MAX_VMAJOR ))
    then
        setNewestDeb $VMAJOR $VMINOR $VMICRO $VBUILD $deb ; continue
    elif (( VMAJOR < MAX_VMAJOR ))
    then
        continue
    fi

    if (( VMINOR > MAX_VMINOR ))
    then
        setNewestDeb $VMAJOR $VMINOR $VMICRO $VBUILD $deb ; continue
    elif (( VMINOR < MAX_VMINOR ))
    then
        continue
    fi

    if (( VMICRO > MAX_VMICRO ))
    then
        setNewestDeb $VMAJOR $VMINOR $VMICRO $VBUILD $deb ; continue
    elif (( VMICRO < MAX_VMICRO ))
    then
        continue
    fi

    if (( VBUILD > MAX_VBUILD ))
    then
        setNewestDeb $VMAJOR $VMINOR $VMICRO $VBUILD $deb ; continue
    elif (( VBUILD < MAX_VBUILD ))
    then
        continue
    fi

done

wget $BUILDS_URL/$MAX_DEB -O /tmp/$MAX_DEB
dpkg -i /tmp/$MAX_DEB
rm -f /tmp/$MAX_DEB

