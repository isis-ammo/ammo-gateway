from ammo import AndroidConnector
import uuid
import sys
import time
import datetime
import math
from ammo import AmmoMessages_pb2

from twisted.internet import reactor

latencies = []

def onDataReceived(connector, msg):
  receivedTime = time.time()
  if msg.type == AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE:
    splitMessage = msg.data_message.data.split("/")
    sequenceNumber = int(splitMessage[0])
    sentTime = float(splitMessage[1])
    timeDifference = receivedTime - sentTime
    print "{0},{1:.9f}".format(sequenceNumber, timeDifference)
    latencies.append(timeDifference)

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 3 and len(sys.argv) != 4:
    print "Usage: ", sys.argv[0], "host port [scope]"
    exit(-1)
  
  scope = AndroidConnector.MessageScope.GLOBAL
  if len(sys.argv) == 4:
    if sys.argv[3] == "local":
      scope = AndroidConnector.MessageScope.LOCAL
    elif sys.argv[3] == "global":
      scope = AndroidConnector.MessageScope.GLOBAL
    else:
      print "scope must be one of: local global"
      exit(-1)
  
  deviceName = "test-consumer"
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector.AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    print "Subscribing."
    connector.subscribe("application/vnd.edu.vu.isis.ammo.test.TestData", scope)
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
