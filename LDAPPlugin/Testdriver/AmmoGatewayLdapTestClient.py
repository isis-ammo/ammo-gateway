
import sys
import socket
import struct
import zlib
import threading
import json

import AmmoMessages_pb2

#==============================================
#
# class AmmoGatewayLdapTestClient
#
#==============================================
class AmmoGatewayLdapTestClient:
    def __init__(self, hostName, port):
        self.Listener = None
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.sock.connect((hostName, int(port)))
        except socket.error, e:
            sys.stderr.write("Error creating socket (%s:%s) -- %s\n"  % (hostName, port, e)  )
            sys.exit(1)
        
        #self.Listener = LdapTestClientListener(self.sock)
        #try:
        #    self.Listener.start()
        #except RuntimeError:
        #    sys.stderr.write("Error starting thread [%s]\n" % self.Listener.getName())
    
    
    def __del__(self):
        #if self.Listener is not None:
        #    self.Listener.KeepGoing = False
        #    try:
        #        self.Listener.join()
        #    except RuntimeError:
        #        sys.stderr.write("Error joining thread [%s]\n" % self.Listener.getName())
        sys.stdout.write("Closing socket.\n")
        self.sock.close()
    
    def sendMessageWrapper(self, msg):
        serializedMsg = msg.SerializeToString()
        #little-endian byte order for now
        self.sock.sendall(struct.pack("<I", len(serializedMsg))) 
        self.sock.sendall(struct.pack("<i", zlib.crc32(serializedMsg)))
        #print serializedMsg
        self.sock.sendall(serializedMsg);
    
    def receiveMessage(self):
        (messageSize,) = struct.unpack("<I", self.sock.recv(4));
        (checksum,) = struct.unpack("<i", self.sock.recv(4));
        protobufMsg = ""
        while (len(protobufMsg) < messageSize) :
            receivedData = self.sock.recv(messageSize - len(protobufMsg))
            protobufMsg += receivedData
        calculatedChecksum = zlib.crc32(protobufMsg)
        if (calculatedChecksum != checksum):
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
        
        # Get authentication response
        response = self.receiveMessage()
        if (response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS):
            sys.stderr.write("Authentication failed...\n")
            return False
        return True
        
    
    def DoPushTest(self):
        #wait for auth response, then send a data push message
        #response = self.receiveMessage()
        #if (response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS):
        #    sys.stderr.write("Authentication failed...\n")
        #    return
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
        m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
        m.data_message.mime_type = "ammo/edu.vu.isis.ammo.launcher.contact"
        
        # The .data element should be a JSON string
        #m.data_message.data = "This is non-JSON text"
        m.data_message.data = self.testJsonString()
        
        sys.stdout.write("Sending data message\n")
        self.sendMessageWrapper(m)
    
    def DoPullTest(self):
        #wait for auth response, then send a data pull message
        #response = self.receiveMessage()
        #if (response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS):
        #    sys.stderr.write("Authentication failed...\n")
        #    return
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
        m.pull_request.request_uid = "contact-req-1"
        m.pull_request.plugin_id = "android-plugin-test"
        m.pull_request.query = "Callsign=Apache*"
        #m.pull_request.query = ""
        m.pull_request.mime_type = "ammo/edu.vu.isis.ammo.launcher.contact_pull"
        sys.stdout.write("Sending pull request...\n")
        self.sendMessageWrapper(m)
    
    def DoSubscribeTest(self):
        #wait for auth response, then send a subscribe message
        #response = self.receiveMessage()
        #if (response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS):
        #    sys.stderr.write("Authentication failed...\n")
        #    return
        m = AmmoMessages_pb2.MessageWrapper()
        m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
        m.subscribe_message.mime_type = "ammo/edu.vu.isis.ammo.battlespace.gcm"
        sys.stdout.write("Sending subscription request...\n")
        self.sendMessageWrapper(m)
    
    #def Stop(self):
    #    self.Listener.Stop()
    
    def testJsonString(self):
        contactEntry = {}
        contactEntry['name'] = "Bob"
        contactEntry['lastname'] = "Smith"
        contactEntry['rank'] = "1Lt"
        contactEntry['callsign'] = "Red Rover"
        contactEntry['branch'] = "Army"
        contactEntry['unitDivision'] = "10MTN"
        contactEntry['unitBrigade'] = "1"
        contactEntry['unitBattalion'] = "3"
        contactEntry['unitCompany'] = "C"
        contactEntry['unitPlatoon'] = "3"
        s = json.dumps(contactEntry)
        sys.stdout.write("*** json-ized string is: %s\n" % s)
        return s


#==============================================
#
# listener class
#
#==============================================
class LdapTestClientListener(threading.Thread):
    def __init__(self, aSocket):
        # Call base threading class constructor (essential to do first)
        threading.Thread.__init__(self)
        
        self.sock = aSocket
        self.KeepGoing = True
        self.name = "blah blah"
    
    # The run() method inherited from Thread... Local version.
    def run(self):
        self.Listen()
    
    def Listen(self):
        msg = self.receiveMessage()
    
    def Stop(self):
        self.KeepGoing = False
    
    def receiveMessage(self):
        (messageSize,) = struct.unpack("<I", self.sock.recv(4));
        (checksum,) = struct.unpack("<i", self.sock.recv(4));
        protobufMsg = ""
        if self.KeepGoing:
            while (len(protobufMsg) < messageSize):
                receivedData = self.sock.recv(messageSize - len(protobufMsg))
                protobufMsg += receivedData
        else:
            return None
        calculatedChecksum = zlib.crc32(protobufMsg)
        if (calculatedChecksum != checksum):
            sys.stderr.write("Checksum error!\n")
            return None
        msg = AmmoMessages_pb2.MessageWrapper()
        msg.ParseFromString(protobufMsg)
        print msg
        return msg
