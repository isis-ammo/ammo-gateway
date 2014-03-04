#!/usr/bin/env python

import uuid
import sys
import time
import datetime
from ammo import AmmoMessages_pb2
from ammo.AndroidConnector import AndroidConnector


from twisted.internet import reactor

if __name__ == "__main__":
  if len(sys.argv) != 3:
    print "Usage: ", sys.argv[0], "host port"
    exit(-1)
  
  deviceName = "device:test/" + uuid.uuid1().hex
  userName = "user:test/" + uuid.uuid1().hex
  
  connector = AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  
  try:
    connector.start()
    
    connector.waitForAuthentication()
    
    print "Publishing messages..."
    for i in range(1,6):
      print "Publishing", i
      connector.push("test:" + str(i) + ":" + uuid.uuid1().hex, "text/plain", "{\"value\": \"" + str(i) + "\"}")
      time.sleep(1)

    print "Querying..."
    connector.pullRequest("text/plain", ",,-4,,", "", 0, 0, False)
    
    while True:
      while(connector.isDataAvailable()):
        result = connector.dequeueMessage()
        if(result != None):
          (msg, receivedTime) = result
          print "Message received at:", receivedTime
          print msg
      time.sleep(0.5)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
    # re-raising the exception so we get a traceback (useful for debugging,
    # occasionally).  Real "ammo"/testdrivers shouldn't do this.
    raise 
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
