#!/bin/bash

WAIT_TIME=10

echo "Killing all gateway processes..."

killall GatewayCore
killall AndroidGatewayPlugin
killall LdapGatewayPlugin
killall DataStoreGatewayPlugin

echo "Waiting $WAIT_TIME seconds to see if processes will exit cleanly..."
sleep $WAIT_TIME

echo "Sending KILL to all gateway processes..."
killall -9 GatewayCore
killall -9 AndroidGatewayPlugin
killall -9 LdapGatewayPlugin
killall -9 DataStoreGatewayPlugin

exit 0
