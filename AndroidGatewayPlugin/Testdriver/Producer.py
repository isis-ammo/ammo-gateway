from ammo import AndroidConnector
import uuid
import sys
import time
import datetime
import os
import optparse
from ammo import AmmoMessages_pb2

from twisted.internet import reactor

def onDataReceived(connector, msg):
  messageType = AmmoMessages_pb2._MESSAGEWRAPPER_MESSAGETYPE.values_by_number[msg.type].name
  print messageType, datetime.datetime.now()

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
  parser.add_option("-r", "--rate", dest="rate", type="float",
                    help="Number of seconds between pushes (default %default)",
                    default=0.5)
  parser.add_option("-m", "--message-size", dest="size", type="int",
                    help="Size of each message's payload (default %default)",
                    default=0)
  
  (options, args) = parser.parse_args()
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector.AndroidConnector(options.gateway, options.port, deviceName, userName, "")
  connector.setMessageQueueEnabled(False)
  connector.registerMessageCallback(onDataReceived)
  
  scope = AndroidConnector.MessageScope.GLOBAL
  if options.scope == "local":
    scope = AndroidConnector.MessageScope.LOCAL
  elif options.scope == "global":
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
      connector.push("item:" + str(sequenceNumber), "ammo/edu.vu.isis.ammo.test.TestData", str(sequenceNumber) + "/" + formattedTime + "/" + os.urandom(options.size), scope)
      print "Pushed sequence number", sequenceNumber, "at", "{0:.6f}".format(time.time())
      sequenceNumber = sequenceNumber + 1
      time.sleep(options.rate)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
