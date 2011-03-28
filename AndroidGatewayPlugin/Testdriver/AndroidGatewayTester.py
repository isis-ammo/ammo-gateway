#!/usr/bin/env python

#Test driver for Android gateway plugin to test deserialization of messages.

import sys
import socket
import struct
import zlib
import time

import AmmoMessages_pb2

class GatewayTestClient:
  def __init__(self, host, port):
    self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.sock.connect((host, int(port)))
    
  def sendMessageWrapper(self, msg):
    serializedMsg = msg.SerializeToString()
    self.sock.sendall(struct.pack("<I", len(serializedMsg))) #little-endian byte order for now
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
      print "Checksum error!"
      return None
    msg = AmmoMessages_pb2.MessageWrapper()
    msg.ParseFromString(protobufMsg)
    return msg

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 4:
    print "Usage:", sys.argv[0], "host port message-type"
    print '''
  where message-type is one of:"
    authenticate : always run, this a dummy actually anything would work.
    subscribe : subscribe to type:edu.vanderbilt.isis.ammo.Test.
    push : send a data message of topic type:edu.vanderbilt.isis.ammo.Test.
	pull : send a pull request to the archiving plugin
'''
    exit(-1)
  
  count = 0
  
  print "Creating client"
  client = GatewayTestClient(sys.argv[1], sys.argv[2])
  print "Generating message"
  m = AmmoMessages_pb2.MessageWrapper()
  m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
  m.authentication_message.device_id = "device:test/device1"
  m.authentication_message.user_id = "user:test/user1"
  m.authentication_message.user_key = "dummy"
  print "Sending message"
  client.sendMessageWrapper(m)
  
  if(sys.argv[3] == "push"):
    #wait for auth response, then send a data push message
    response = client.receiveMessage()
    if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
      print "Authentication failed..."
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
    m.data_message.mime_type = "text/plain"
    m.data_message.data = "This is some text being pushed out to the gateway." + str(count)
    count = count + 1
    print "Sending data message"
    client.sendMessageWrapper(m)
  elif sys.argv[3] == "subscribe": 
    #wait for auth response, then send a data push message
    response = client.receiveMessage()
    if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
      print "Authentication failed..."
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = "text/plain"
    print "Sending subscription request..."
    client.sendMessageWrapper(m)
  elif sys.argv[3] == "pull":
    #wait for auth response, then send a pull request message
    response = client.receiveMessage()
    if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
      print "Authentication failed..."    
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
    m.pull_request.mime_type = "text/plain"
    m.pull_request.request_uid = "AGT_pull_request"
    m.pull_request.plugin_id = "AndroidGatewayTester"
    m.pull_request.max_results = 0
    m.pull_request.query = ",,1298478000,1300000000"
    print "Sending pull request..."
    client.sendMessageWrapper(m)
  
  while True:
    msg = client.receiveMessage()
    print msg
    
    if(sys.argv[3] == "push"):
      time.sleep(0.5)
      m = AmmoMessages_pb2.MessageWrapper()
      m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
      m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
      m.data_message.mime_type = "text/plain"
      m.data_message.data = "This is some text being pushed out to the gateway." + str(count)
      count = count + 1
      print "Sending data message"
      client.sendMessageWrapper(m)
  
  print "Closing socket"
  
