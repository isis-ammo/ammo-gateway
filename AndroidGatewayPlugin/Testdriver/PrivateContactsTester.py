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
#  userName = "kyle.anderson"
  
  connector = AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
  
  try:
    connector.start()
    connector.waitForAuthentication()
    
    mimeType = "ammo/edu.vu.isis.ammo.private_contacts"
    
    print "Subscribing to type ", mimeType
    
    uid = "type:edu.vanderbilt.isis.ammo.Test"
    
    firstName = "Jimmy"
    middleInitial = "I"
    lastName = "Bork"
    n = 1
    
    # Send 5 text message pushes, each one with a 'rank' string containing the
    # index. This field is what the pull request fill filter on. The data message
    # emulates the JSON encoding of the private contacts data format.
    while n <= 5:
      time.sleep(0.5)
      data = "{\"first_name\":\"" + firstName + "\", "
      data += "\"middle_initial\":\"" + middleInitial + "\", "
      data += "\"last_name\":\"" + lastName + "\", "
      data += "\"rank\":\"E" + str(n) + "\", "
      data += "\"call_sign\":\"\", "
      data += "\"branch\":\"\", "
      data += "\"unit\":\"\", "
      data += "\"email\":\"\", "
      data += "\"phone\":\"\" }"
      output_msg = "Sending data message " + str(n)
      print output_msg
      connector.push(uid, mimeType, data)
      n += 1
	
    # Send a single pull request.
    
    # Only 1 of the 5 messages sent will match this, so we'll get 1 match
    # for every time this test has been run on a given gateway.
    rank = "E4"
    
    call_sign = ""
    branch = ""
    unit = ""
    email = ""
    phone = ""
    
    query = userName + ","
    query += uid + ","
    query += firstName + ","
    query += middleInitial + ","
    query += lastName + ","
    query += rank + ","
    query += call_sign + ","
    query += branch + ","
    query += phone + ","
    query += email + ","
    
    # Private contacts data is not stored as a blob so we don't use a projection string
    projection = ""
  
    maxResults = 0
    startFromCount = 0
    liveQuery = False

    connector.pullRequest(mimeType, query, projection, maxResults, startFromCount, liveQuery)
    
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
    # occasionally).  Real "applications"/testdrivers shouldn't do this.
    raise 
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
