while true ; do
python AndroidGatewayTester.py localhost 32869 subscribe &
sleep 3;
kill %+
done
