import sys
import time
import datetime
import math

latencies = []


if __name__ == "__main__":
  print "Java API Tester"

  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/dist/lib/gatewaypluginapi-20120227.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/json-20090211.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/slf4j-api-1.6.4.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/slf4j-simple-1.6.4.jar")
  sys.path.append("/home/ammmo/ammo/Gateway/JavaGatewayConnector/libs/protobuf.jar")

  from edu.vu.isis.ammo.gateway import GatewayConnector
  from edu.vu.isis.ammo.gateway import DataPushReceiverListener
  from edu.vu.isis.ammo.gateway import PushData
  from org.json import JSONException


  class DataPushReceiver(DataPushReceiverListener):
    def __init__(self):
      print "Constructor"

    def onPushDataReceived(self, sender, data):
      receivedTime = time.time()
      print data.uri
      print data.mimeType


  
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
  

  connector = GatewayConnector( None )
  receiver  = DataPushReceiver( )
  
  try:
    time.sleep(3)
    print "Subscribing."
    # register a data listener
    connector.registerDataInterest("ammo/edu.vu.isis.ammo.dash.event", receiver, None);

    while True:
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
