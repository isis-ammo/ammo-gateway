#!/bin/bash

LOGDIR="/tmp/gatewaylogs"

if pgrep -l slapd > /dev/null 2>&1 ; then
  echo "LDAP server: running"
else
  echo "ERROR:  LDAP server is not running...  run it with"
  echo "'sudo ~/ldap/ldap.sh', then rerun this script."
  exit 1
fi

if [ ! -d $LOGDIR ]; then
  mkdir $LOGDIR
fi

hostname=`hostname`
datesuffix=`date "+%Y%m%d.%H%M%S"`

gatewaycorelog="$LOGDIR/GatewayCore.log.$datesuffix"
androidpluginlog="$LOGDIR/AndroidGatewayPlugin.log.$datesuffix"
datastorepluginlog="$LOGDIR/DataStoreGatewayPlugin.log.$datesuffix"
ldappluginlog="$LOGDIR/LdapGatewayPlugin.log.$datesuffix"
passpluginlog="$LOGDIR/PassGatewayPlugin.log.$datesuffix"

echo "Launching Gateway Core..."
echo "  Log file in $gatewaycorelog"
GatewayCore > $gatewaycorelog 2>&1 &

sleep 5

echo "Launching Android Gateway Plugin..."
echo "  Log file in $androidpluginlog"
AndroidGatewayPlugin --listenPort 33289 > $androidpluginlog 2>&1 &

sleep 5

echo "Launching Data Store Gateway Plugin..."
echo "  Log file in $datastorepluginlog"
DataStoreGatewayPlugin > $datastorepluginlog 2>&1 &

sleep 5

echo "Launching LDAP Gateway Plugin..."
echo "  Log file in $ldappluginlog"
LdapGatewayPlugin > $ldappluginlog 2>&1 &

sleep 5

echo "Launching Pass Gateway Plugin..."
echo "  Log file in $passpluginlog"
PassGatewayPlugin > $passpluginlog 2>&1 &

echo "Gateway is started...  run ./kill_all_gateway.sh to stop."
