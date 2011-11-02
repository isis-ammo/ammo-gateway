#! /bin/sh
#
# ammo-gateway	/etc/init.d/ startup and shutdown script.
#
# Version:	1.0  03-May-2011  johnwilliams@isis.vanderbilt.edu
#

### BEGIN INIT INFO
# Provides:          ammo-gateway
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Start daemon at boot time
# Description:       Enable service provided by daemon.
### END INIT INFO

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DESC="AMMO Gateway"

# Include ammo-gateway defaults if available
if [ -f /etc/default/ammo-gateway ] ; then
    . /etc/default/ammo-gateway
fi

set -e

isProcessRunning()
{
    pgrepable=`echo $1 | cut -c1-15`
    if [ ! "`pgrep $pgrepable | wc -l`" -eq "0" ]
    then
        echo $1 is running
    else
        echo $1 is NOT running
    fi
}

startGateway()
{
    /usr/bin/launch_ammo_gateway_headless.sh
}

stopGateway()
{
    /usr/bin/kill_all_gateway.sh
}

case "$1" in
  start)
        echo "Starting $DESC: "
        startGateway
        ;;
  stop)
        echo "Stopping $DESC: "
        stopGateway
        ;;
  restart)
    echo "Restarting $DESC: "
        stopGateway
        startGateway
        ;;
  status)
    isProcessRunning GatewayCore
    isProcessRunning AndroidGatewayPlugin
    isProcessRunning LdapGatewayPlugin
    isProcessRunning DataStoreGatewayPlugin
    isProcessRunning PassGatewayPlugin
    ;;
  *)
    N=/etc/init.d/$NAME
    echo "Usage: $N {start|stop|restart|status}" >&2
    exit 1
    ;;
esac

exit 0
