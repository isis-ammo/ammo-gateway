#!/bin/bash

LOGDIR="/var/log/ammo-gateway"

if pgrep -l slapd > /dev/null 2>&1 ; then
  echo "LDAP server: running"
else
  echo "WARNING:  LDAP server is not running"
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
mcastpluginlog="$LOGDIR/MCastGatewayPlugin.log.$datesuffix"
rmcastpluginlog="$LOGDIR/RMCastGatewayPlugin.log.$datesuffix"

echo "Launching Gateway Core..."
echo "  Log file in $gatewaycorelog"
GatewayCore > $gatewaycorelog 2>&1 &

sleep 5

echo "Launching Data Store Gateway Plugin..."
echo "  Log file in $datastorepluginlog"
DataStoreGatewayPlugin > $datastorepluginlog 2>&1 &

sleep 2

echo "Launching Android Gateway Plugin..."
echo "  Log file in $androidpluginlog"
AndroidGatewayPlugin --listenPort 33289 > $androidpluginlog 2>&1 &

sleep 2

echo "Launching Multicast Plugin..."
echo "  Log file in $mcastpluginlog"
mcastplugin.sh >$mcastpluginlog 2>&1 &

sleep 2

echo "Launching Reliable Multicast Plugin..."
echo "  Log file in $rmcastpluginlog"
rmcastplugin.sh >$rmcastpluginlog 2>&1 &

#sleep 5
#
#echo "Launching LDAP Gateway Plugin..."
#echo "  Log file in $ldappluginlog"
#LdapGatewayPlugin > $ldappluginlog 2>&1 &

echo "Gateway is started...  run ./kill_all_gateway.sh to stop."
