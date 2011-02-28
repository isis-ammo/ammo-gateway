#!/usr/bin/env python 

"""

Client-side test driver for LDAP gateway plugin

"""

import sys
import socket
import struct
import zlib
import AmmoMessages_pb2

#==============================================
#
# class GatewayTestClient
#
#==============================================
class GatewayTestClient:
  HEADER_MAGIC_NUMBER = 0xfeedbeef
    def __init__(self, host, port):
      self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
      self.sock.connect((host, int(port)))
      
    def sendMessageWrapper(self, msg):
      serializedMsg = msg.SerializeToString()
      messageHeader = struct.pack("<IIi", self.HEADER_MAGIC_NUMBER, len(serializedMsg), zlib.crc32(serializedMsg))
      self.sock.sendall(messageHeader) #little-endian byte order for now
      self.sock.sendall(struct.pack("<i", zlib.crc32(messageHeader)))
      print serializedMsg
      self.sock.sendall(serializedMsg)
      
    def receiveMessage(self):
      messageHeader = self.sock.recv(3*4)
      (magicNumber, messageSize, checksum) = struct.unpack("<IIi", messageHeader)
      (headerChecksum,) = struct.unpack("<i", self.sock.recv(4))
      
      if magicNumber != self.HEADER_MAGIC_NUMBER:
        raise IOError("Invalid magic number received from gateway: " + hex(magicNumber))
        
      if headerChecksum != zlib.crc32(messageHeader):
        raise IOError("Invalid header checksum received from gateway")
      
      protobufMsg = ""
      while len(protobufMsg) < messageSize:
        receivedData = self.sock.recv(messageSize - len(protobufMsg))
        protobufMsg += receivedData
      calculatedChecksum = zlib.crc32(protobufMsg)
      if calculatedChecksum != checksum:
        print "Checksum error!"
        return None
      msg = AmmoMessages_pb2.MessageWrapper()
      msg.ParseFromString(protobufMsg)
      return msg

#==============================================
#
# EnforceUsage()
#
#==============================================
def EnforceUsage():
    if len(sys.argv) != 4:
        sys.stderr.write("Usage: %s <host> <port> <message-type>\n" % sys.argv[0])
        helpString = '''Where <message-type> is one of:
        authenticate : always run, this a dummy actually anything would work.
        subscribe : subscribe to type:edu.vanderbilt.isis.ammo.Test.
        pull : search for type:edu.vu.isis.ammo.launhcer.contact_pull.
        push : send a data message of topic type:edu.vanderbilt.isis.ammo.Test.'''
        sys.stderr.write("\n%s\n\n" % helpString)
        sys.exit(-1)
    

#==============================================
#
# main()
#
#==============================================
def main():
    sys.stdout.write('Gateway LDAP plugin test\n')
    EnforceUsage()
    
    sys.stdout.write("Creating client\n")
    client = GatewayTestClient(sys.argv[1], sys.argv[2])
    sys.stdout.write("Generating message\n")
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
    m.authentication_message.device_id = "device:test/device1"
    m.authentication_message.user_id = "user:test/user1"
    m.authentication_message.user_key = "dummy"
    sys.stdout.write("Sending message\n")
    client.sendMessageWrapper(m)
    
    if(sys.argv[3] == "push"):
        #wait for auth response, then send a data push message
        response = client.receiveMessage()
        if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
            sys.stderr.write("Authentication failed...\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
        m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
        m.data_message.mime_type = "text/plain"
        m.data_message.data = "This is some text being pushed out to the gateway."
        sys.stdout.write("Sending data message\n")
        client.sendMessageWrapper(m)
    elif sys.argv[3] == "subscribe":
        #wait for auth response, then send a subscribe message
        response = client.receiveMessage()
        if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
            sys.stderr.write("Authentication failed...\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
        m.subscribe_message.mime_type = "application/vnd.edu.vu.isis.ammo.battlespace.gcm"
        sys.stdout.write("Sending subscription request...\n")
        client.sendMessageWrapper(m)
    elif sys.argv[3] == "pull":
        #wait for auth response, then send a data pull message
        response = client.receiveMessage()
        if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
            sys.stderr.write("Authentication failed...\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
        m.pull_request.request_uid = "contact-req-1"
        m.pull_request.plugin_id = "android-plugin-test"
        #m.pull_request.query = "x-Callsign = Hawk*"
        m.pull_request.query = ""
        m.pull_request.mime_type = "application/vnd.edu.vu.isis.ammo.launcher.contact_pull"
        sys.stdout.write("Sending pull request...\n")
        client.sendMessageWrapper(m)
        
    try:
        while True:
            msg = client.receiveMessage()
            print msg
    except KeyboardInterrupt:
        sys.stdout.write("Closing socket.\n")

#---------------------------

if (__name__ == "__main__"):
    EnforceUsage()
    main()
