#!/bin/bash

while true ; do
python AndroidGatewayTester.py localhost 33289 push &
sleep 3;
kill %+
done
