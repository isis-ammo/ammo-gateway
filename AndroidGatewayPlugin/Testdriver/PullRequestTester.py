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
    
    mimeType = "application/vnd.edu.vu.isis.ammo.sms.message"
    
    print "Subscribing to type ", mimeType
    
    uri = "type:edu.vanderbilt.isis.ammo.Test"
      
    n = 1
    
    # Plug this value into the time_start slot in the query string. The value comes
    # from the underlying Unix gettimeofday(), which is also used by the ACE version
    # that stores the timestamp in the database.
    start = int(time.time())
    
    # Send 5 text message pushes, each one with a recipient string containing the
    # index. This field is what the pull request fill filter on. The data message
    # emulates the JSON encoding of the SMS data format.
    while n <= 5:
      time.sleep(0.5)
      data = "{ \"sms_uri\" : \"" + uri + "\", "
      data += "\"sender\" : \"sgt_kill_em_all@151st.mil\", "
      data += "\"recipient\" : \"soldier_" + str(n) + "\", "
      data += "\"thread\" : 14, "
      data += "\"payload\" : \"hey soldier_" + str(n) + ", meet me at the PX\", "
      data += "\"createdDate\" : " + str(start) + ", "
      data += "\"modifiedDate\" : " + str(start)
      data += " }"
      output_msg = "Sending data message " + str(n)
      print output_msg
      connector.push(uri, mimeType, data)
      n += 1
	
    # max_results is unlimited, but the use of the start time string
    # will restrict responses to be from among the 5 pushes above, no matter
    # how many times this test has been previously run on a given gateway.
    query = ",," + str(start) + ",,"
    
    # This string will filter the responses to a single message.
    projection = ",,soldier_2,,,,,,,"
  
    connector.pullRequest(mimeType, query, projection, 0, 0, False)
    
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
