import AndroidConnector
import uuid
import sys
import time
import datetime
import AmmoMessages_pb2

from twisted.internet import reactor

def onDataReceived(connector, msg):
  messageType = AmmoMessages_pb2._MESSAGEWRAPPER_MESSAGETYPE.values_by_number[msg.type].name
  print messageType, datetime.datetime.now()

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 4 and len(sys.argv) != 5:
    print "Usage: ", sys.argv[0], "host port rate [scope]"
    exit(-1)
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector.AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  scope = AndroidConnector.MessageScope.GLOBAL
  if len(sys.argv) == 5:
    if sys.argv[4] == "local":
      scope = AndroidConnector.MessageScope.LOCAL
    elif sys.argv[4] == "global":
      scope = AndroidConnector.MessageScope.GLOBAL
    else:
      print "scope must be one of: local global"
      exit(-1)
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    print "Pushing a lot of data."
    sequenceNumber = 0
    
    while True:
      postTime = time.time()
      formattedTime = "{0:.6f}".format(postTime)
      connector.push("item:" + str(sequenceNumber), "application/vnd.edu.vu.isis.ammo.test.TestData", str(sequenceNumber) + "/" + formattedTime, scope)
      print "Pushed sequence number", sequenceNumber, "at", "{0:.6f}".format(time.time())
      sequenceNumber = sequenceNumber + 1
      time.sleep(float(sys.argv[3]))
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
