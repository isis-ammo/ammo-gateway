#!/usr/bin/env python 

"""

Client-side test driver for LDAP gateway plugin

"""

import sys
from AmmoGatewayLdapTestClient import *

#==============================================
# main()
#==============================================
def main():
    sys.stdout.write('Gateway LDAP plugin test\n')
    
    # Enforce usage
    if len(sys.argv) != 4:
        sys.stderr.write("Usage: %s <host> <port> <message-type>\n" % sys.argv[0])
        helpString = '''Where <message-type> is one of:
        authenticate : always run, this a dummy actually anything would work.
        subscribe : subscribe to type:edu.vanderbilt.isis.ammo.Test.
        pull : search for type:edu.vu.isis.ammo.launhcer.contact_pull.
        push : send a data message of topic type:edu.vanderbilt.isis.ammo.Test.'''
        sys.stderr.write("\n%s\n\n" % helpString)
        sys.exit(-1)
    
    sys.stdout.write("Creating client\n")
    client = AmmoGatewayLdapTestClient(sys.argv[1], sys.argv[2])
    client.EstablishGatewayConnection()
    
    if(sys.argv[3] == "push"):
        client.DoPushTest()
    elif sys.argv[3] == "subscribe":
        client.DoSubscribeTest()
    elif sys.argv[3] == "pull":
        client.DoPullTest()
    
    try:
        while True:
            msg = client.receiveMessage()
            print msg
    except KeyboardInterrupt:
        sys.stdout.write("Closing socket.\n")

#---------------------------

if (__name__ == "__main__"):
    main()
