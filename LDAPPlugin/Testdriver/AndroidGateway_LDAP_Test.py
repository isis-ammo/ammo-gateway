#! /usr/bin/python

"""

Test driver for LDAP gateway plugin

"""

import sys
import uuid
import time
import datetime
from ammo import AmmoMessages_pb2
from ammo.AndroidConnector import AndroidConnector

from twisted.internet import reactor

#==============================================
# main()
#==============================================
def main():
    sys.stdout.write("Android Gateway Pull Test\n")
    if len(sys.argv) != 3:
        sys.stderr.write("Usage: %s <host> <port> \n" % sys.argv[0])
        sys.exit(-1)
    
    deviceName = "device:test/" + uuid.uuid1().hex
    userName = "user:test/" + uuid.uuid1().hex
    connector = AndroidConnector(sys.argv[1], int(sys.argv[2]), deviceName, userName, "")
    
    try:
        connector.start()
        connector.waitForAuthentication()

        # Pull request options... For LDAP pull, only the first two are important (as of now)
        mimetype = "ammo/edu.vu.isis.ammo.launcher.contact_pull"
        query = "cn=John"
        maxResults = 0
        startFromCount = 0
        liveQuery = False
        
        # Send the pull request
        connector.pullRequest(mimetype, query, "", maxResults, startFromCount, liveQuery)
        
        while True:
            while(connector.isDataAvailable()):
                result = connector.dequeueMessage()
                if(result != None):
                    (msg, receivedTime) = result
                    print "Message received at:", receivedTime
                    print msg
            time.sleep(0.5)
    except KeyboardInterrupt:
        sys.stdout.write("Got ^C...  Closing\n")
        reactor.callFromThread(reactor.stop)
    except:
        sys.stderr.write("Unexpected error...  dying.\n")
        reactor.callFromThread(reactor.stop)
        raise

#---------------------------

if (__name__ == "__main__"):
    main()
