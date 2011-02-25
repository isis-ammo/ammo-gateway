#!/bin/bash

while true ; do
python AndroidGatewayTester.py localhost 32869 push &
sleep 3;
kill %+
done
