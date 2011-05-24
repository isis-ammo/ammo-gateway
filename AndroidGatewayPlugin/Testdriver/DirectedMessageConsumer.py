import AndroidConnector
import uuid
import sys
import time
import datetime
import AmmoMessages_pb2

from twisted.internet import reactor

def onDataReceived(connector, msg):
  receivedTime = time.time()
  if msg.type == AmmoMessages_pb2.MessageWrapper.DIRECTED_MESSAGE:
    sequenceNumber = int(msg.directed_message.data.split("/")[0])
    sentTime = float(msg.directed_message.data.split("/")[1])
    timeDifference = receivedTime - sentTime
    print "{0},{1:.9f} from {2}".format(sequenceNumber, timeDifference, msg.directed_message.origin_user)

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
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/DirectedMessageConsumer"
  
  connector = AndroidConnector.AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
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
