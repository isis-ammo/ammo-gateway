from ammo import AndroidConnector
from ammo import AmmoMessages_pb2

import uuid
import sys
import time
import datetime
import math
import optparse


from twisted.internet import reactor

latencies = []

def onDataReceived(connector, msg):
  print msg
  receivedTime = time.time()
  #if msg.type == AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE:
  #  splitMessage = msg.data_message.data.split("/")
  #  sequenceNumber = int(splitMessage[0])
  #  sentTime = float(splitMessage[1])
  #  timeDifference = receivedTime - sentTime
  #  print "{0},{1:.9f}".format(sequenceNumber, timeDifference)
  #  latencies.append(timeDifference)

if __name__ == "__main__":
  print "Android Gateway Tester"
  
  parser = optparse.OptionParser()
  parser.add_option("-g", "--gateway", dest="gateway",
                    help="Gateway to connect to (default %default)",
                    default="127.0.0.1")
  parser.add_option("-p", "--port", dest="port", type="int",
                    help="Gateway port to connect to (default %default)",
                    default=33289)
  parser.add_option("-s", "--scope", dest="scope",
                    help="Subscription scope (either local or global; default %default)",
                    default="global")
  parser.add_option("-b", "--heartbeat-period", dest="heartbeatPeriod", type="float",
                    help="Time between heartbeats (default %default)",
                    default=30)
  
  (options, args) = parser.parse_args()
  
  scope = AndroidConnector.MessageScope.GLOBAL
  if options.scope == "local":
    scope = AndroidConnector.MessageScope.LOCAL
  elif options.scope == "global":
    scope = AndroidConnector.MessageScope.GLOBAL
  else:
    print "scope must be one of: local global"
    exit(-1)
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector.AndroidConnector(options.gateway, options.port, deviceName, userName, "", options.heartbeatPeriod)
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    print "Subscribing."
    connector.subscribe("ammo/edu.vu.isis.ammo.dash.event", scope)
    connector.subscribe("ammo/edu.vu.isis.ammo.dash.media", scope)
    sequenceNumber = 0
    
    while True:
      time.sleep(0.5)
      
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
