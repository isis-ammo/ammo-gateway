#!/bin/bash

LOGDIR="/tmp/gatewaylogs"

export GATEWAY_ROOT="$PWD"
export LD_LIBRARY_PATH=$GATEWAY_ROOT/lib:$LD_LIBRARY_PATH

#if pgrep -l slapd > /dev/null 2>&1 ; then
#  echo "LDAP server: running"
#else
#  echo "ERROR:  LDAP server is not running...  run it with"
#  echo "'sudo ~/ldap/ldap.sh', then rerun this script."
#  exit 1
#fi

if [ ! -d $LOGDIR ]; then
  mkdir $LOGDIR
fi

hostname=`hostname`
datesuffix=`date "+%Y.%m.%d.%H.%M.%S"`

gatewaycorelog="$LOGDIR/GatewayCore.log.$datesuffix"
androidpluginlog="$LOGDIR/AndroidGatewayPlugin.log.$datesuffix"
tigrpluginlog="$LOGDIR/TigrGatewayPlugin.log.$datesuffix"
passpluginlog="$LOGDIR/PassGatewayPlugin.log.$datesuffix"
ldappluginlog="$LOGDIR/LdapGatewayPlugin.log.$datesuffix"

echo "Launching Gateway Core..."
echo "  Log file in $gatewaycorelog"
GatewayCore > $gatewaycorelog 2>&1 &
xterm -bg black -fg orange -sb -title "Gateway Core ($HOSTNAME)" -e "tail -n+0 -f $gatewaycorelog" &

sleep 5

echo "Launching Android Gateway Plugin..."
echo "  Log file in $androidpluginlog"
AndroidGatewayPlugin --listenPort 33289 > $androidpluginlog 2>&1 &
xterm -bg black -fg cyan -sb -title "Android Gateway Plugin ($HOSTNAME)" -e "tail -n+0 -f $androidpluginlog" &

sleep 5

echo "Launching TIGR Gateway Plugin..."
echo "  Log file in $tigrpluginlog"
TigrGatewayPlugin > $tigrpluginlog 2>&1 &
xterm -bg black -fg yellow -sb -title "TIGR Gateway Plugin ($HOSTNAME)" -e "tail -n+0 -f $tigrpluginlog" &

sleep 5

echo "Launching PASS Gateway Plugin..."
echo "  Log file in $passpluginlog"
PassGatewayPlugin > $passpluginlog 2>&1 &
xterm -bg black -fg gray -sb -title "PASS Gateway Plugin ($HOSTNAME)" -e "tail -n+0 -f $passpluginlog" &

sleep 5

echo "Launching LDAP Gateway Plugin..."
echo "  Log file in $ldappluginlog"
LdapGatewayPlugin > $ldappluginlog 2>&1 &
echo ""
echo "Type Control+C to exit."
xterm -bg black -fg green -sb -title "LDAP Gateway Plugin ($HOSTNAME)" -e "tail -n+0 -f $ldappluginlog"

echo "Terminating gateway applications..."

echo "Terminating Gateway Core..."
kill %Gatewa
echo "Terminating Android Gateway Plugin..."
kill %AndroidGat
echo "Terminating TIGR Gateway Plugin..."
kill %TigrGat
echo "Terminating PASS Gateway Plugin..."
kill %PassGat
echo "Terminating LDAP Gateway Plugin..."
kill %LdapGat
