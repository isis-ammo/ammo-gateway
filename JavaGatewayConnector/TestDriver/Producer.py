import sys
import time
import datetime
import math

latencies = []


if __name__ == "__main__":
  print "Java API Tester"
  
  # parser = optparse.OptionParser()
  # parser.add_option("-g", "--gateway", dest="gateway",
  #                   help="Gateway to connect to (default %default)",
  #                   default="127.0.0.1")
  # parser.add_option("-p", "--port", dest="port", type="int",
  #                   help="Gateway port to connect to (default %default)",
  #                   default=32869)
  # parser.add_option("-s", "--scope", dest="scope",
  #                   help="Subscription scope (either local or global; default %default)",
  #                   default="global")
  
  # (options, args) = parser.parse_args()
  
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/dist/lib/gatewaypluginapi-20120227.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/json-20090211.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/slf4j-api-1.6.4.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/slf4j-simple-1.6.4.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/protobuf.jar")

  from edu.vu.isis.ammo.gateway import GatewayConnector
  from edu.vu.isis.ammo.gateway import PushData
  from org.json import JSONException

  connector = GatewayConnector( None )
  data = PushData( )
  data.uri = "java api test"
  data.mimeType = "ammo/edu.vu.isis.ammo.dash.event"
  data.originUserName = "testDriver"
  data.data = "My BIG FAT JSON String"
  
  try:
    time.sleep(3)

    while True:
      print "Pushing Data."
      ret = connector.pushData(data)
      print ret
      time.sleep(5)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
    print "MeanLatency,StdDev,Samples"
    if len(latencies) > 0:
      meanLatency = sum(latencies) / len(latencies)
      squaredDifferences = [(x - meanLatency) ** 2 for x in latencies]
      variance = sum(squaredDifferences) / len(latencies)
      stddev = math.sqrt(variance)
      print str(meanLatency) + "," + str(stddev) + "," + str(len(latencies))
    else:
      print "0,0"
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
