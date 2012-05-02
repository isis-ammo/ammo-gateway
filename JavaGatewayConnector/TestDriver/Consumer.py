import sys
import time
import datetime
import math
import os

repo_dir = os.getenv("AMMO_REPO_DIR");
if repo_dir is None:
   head,tail = os.path.split(os.getcwd())
   while tail != "JavaGatewayConnector":
      head,tail = os.path.split(head)
      if not tail:
         print "not proper directory " + jar_dir
         sys.exit
   jar_dir = os.path.join(head,"JavaGatewayConnector")
else:
   jar_dir = repo_dir+"/Gateway/JavaGatewayConnector"

if not os.path.isdir(jar_dir):
   print "not a directory " + jar_dir
   sys.exit

latencies = []


if __name__ == "__main__":
  print "Java API Tester"
  print jar_dir

  sys.path.append(jar_dir+"/dist/lib/gatewaypluginapi-1.3.9.jar")
  sys.path.append(jar_dir+"/libs/json-20090211.jar")
  sys.path.append(jar_dir+"/libs/slf4j-api-1.6.4.jar")
  sys.path.append(jar_dir+"/libs/slf4j-simple-1.6.4.jar")
  sys.path.append(jar_dir+"/libs/protobuf.jar")

  from edu.vu.isis.ammo.gateway import GatewayConnector
  from edu.vu.isis.ammo.gateway import GatewayConnectorDelegate
  
  from edu.vu.isis.ammo.gateway import DataPushReceiverListener
  from edu.vu.isis.ammo.gateway import PushData
  from org.json import JSONException

  class DataPushReceiver(DataPushReceiverListener):
    def __init__(self):
      print "DataPushReceiverListener.<constructor>"

    def onPushDataReceived(self, sender, data):
      print "DataPushReceiverListener.onPushDataReceived"
      receivedTime = time.time()
      print receivedTime
      print data.uri
      print data.originUserName
      print data.mimeType

  class GatewayConnectorD(GatewayConnectorDelegate):
    def __init__(self):
      print "GatewayConnectorDelegate.<constructor>"

    def onConnect(self, sender):
      print "GatewayConnectorDelegate.onConnect"
      print "Subscribing."
      receiver  = DataPushReceiver( )
      sender.registerDataInterest("ammo/transapps.chat.message_groupAll", receiver, None);

    def onDisconnect(self, sender):
      print "GatewayConnectorDelegate.onDisonnect"

    def onAuthenticationResponse(self, sender, result):
      print "GatewayConnectorDelegate.onAuthenticationResponse"

      

  
  # parser = optparse.OptionParser()
  # parser.add_option("-g", "--gateway", dest="gateway",
  #                   help="Gateway to connect to (default %default)",
  #                   default="127.0.0.1")
  # parser.add_option("-p", "--port", dest="port", type="int",
  #                   help="Gateway port to connect to (default %default)",
  #                   default=33289)
  # parser.add_option("-s", "--scope", dest="scope",
  #                   help="Subscription scope (either local or global; default %default)",
  #                   default="global")
  
  # (options, args) = parser.parse_args()
  
  delegate  = GatewayConnectorD( )
  connector = GatewayConnector( delegate )
  
  try:
    while True:
      time.sleep(5)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    print "MeanLatency,StdDev,Samples"
    if len(latencies) > 0:
      meanLatency = sum(latencies) / len(latencies)
      squaredDifferences = [(x - meanLatency) ** 2 for x in latencies]
      variance = sum(squaredDifferences) / len(latencies)
      stddev = math.sqrt(variance)
      print str(meanLatency) + "," + str(stddev) + "," + str(len(latencies))
    else:
      print "0,0"
