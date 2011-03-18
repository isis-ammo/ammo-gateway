import AndroidConnector
import uuid
import sys
import time
import datetime
import AmmoMessages_pb2

from twisted.internet import reactor

def onDataReceived(connector, msg):
  receivedTime = time.time()
  messageType = AmmoMessages_pb2._MESSAGEWRAPPER_MESSAGETYPE.values_by_number[msg.type].name
  print messageType, receivedTime,
  if msg.type == AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE:
    sentTime = float(msg.data_message.data.split("/")[1])
    timeDifference = receivedTime - sentTime
    print "::", msg.data_message.data, "::", timeDifference
  else:
    print ""

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 3:
    print "Usage: ", sys.argv[0], "host port"
    exit(-1)
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector.AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    print "Subscribing."
    connector.subscribe("application/vnd.edu.vu.isis.ammo.test.TestData")
    sequenceNumber = 0
    
    while True:
      time.sleep(0.5)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
