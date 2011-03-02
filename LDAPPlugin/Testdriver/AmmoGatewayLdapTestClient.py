
import sys
import socket
import struct
import zlib

import AmmoMessages_pb2

#==============================================
#
# class AmmoGatewayLdapTestClient
#
#==============================================
class AmmoGatewayLdapTestClient:
    def __init__(self, hostName, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((hostName, int(port)))
        except socket.error, e:
            sys.stderr.write("Error creating socket (%s:%s) -- %s\n"  % (hostName, port, e)  )
            sys.exit(1)
    
    def __del__(self):
        self.sock.close()
    
    def sendMessageWrapper(self, msg):
        serializedMsg = msg.SerializeToString()
        #little-endian byte order for now
        self.sock.sendall(struct.pack("<I", len(serializedMsg))) 
        self.sock.sendall(struct.pack("<i", zlib.crc32(serializedMsg)))
        print serializedMsg
        self.sock.sendall(serializedMsg);
  
    def receiveMessage(self):
        (messageSize,) = struct.unpack("<I", self.sock.recv(4));
        (checksum,) = struct.unpack("<i", self.sock.recv(4));
        protobufMsg = ""
        while len(protobufMsg) < messageSize:
            receivedData = self.sock.recv(messageSize - len(protobufMsg))
            protobufMsg += receivedData
        calculatedChecksum = zlib.crc32(protobufMsg)
        if calculatedChecksum != checksum:
            sys.stderr.write("Checksum error!\n")
            return None
        msg = AmmoMessages_pb2.MessageWrapper()
        msg.ParseFromString(protobufMsg)
        return msg
    
    def EstablishGatewayConnection(self):
        sys.stdout.write("Generating message\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
        m.authentication_message.device_id = "device:test/device1"
        m.authentication_message.user_id = "user:test/user1"
        m.authentication_message.user_key = "dummy"
        sys.stdout.write("Sending message\n")
        self.sendMessageWrapper(m)
    
    def DoPushTest(self):
        #wait for auth response, then send a data push message
        response = self.receiveMessage()
        if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
            sys.stderr.write("Authentication failed...\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
        m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
        m.data_message.mime_type = "text/plain"
        m.data_message.data = "This is some text being pushed out to the gateway."
        sys.stdout.write("Sending data message\n")
        self.sendMessageWrapper(m)
    
    def DoPullTest(self):
        #wait for auth response, then send a data pull message
        response = self.receiveMessage()
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
        self.sendMessageWrapper(m)
    
    def DoSubscribeTest(self):
        #wait for auth response, then send a subscribe message
        response = self.receiveMessage()
        if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
            sys.stderr.write("Authentication failed...\n")
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
        m.subscribe_message.mime_type = "application/vnd.edu.vu.isis.ammo.battlespace.gcm"
        sys.stdout.write("Sending subscription request...\n")
        self.sendMessageWrapper(m)


