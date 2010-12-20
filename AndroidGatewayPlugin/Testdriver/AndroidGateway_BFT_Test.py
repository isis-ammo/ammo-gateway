#!/usr/bin/env python

#Test driver for Android gateway plugin to test deserialization of messages.

import sys
import socket
import struct
import zlib

import DataMessage_pb2

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
    msg = DataMessage_pb2.MessageWrapper()
    msg.ParseFromString(protobufMsg)
    return msg

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 4:
    print "Usage:", sys.argv[0], "host port message-type"
    print '''
 where message-type is one of:"
  authenticate : this is always sent regardless of what you request. 
       however as you must send some message this type is provided.
  initial : a message sending a single unit and its location"
'''
    exit(-1)
  
  print "Creating client"
  client = GatewayTestClient(sys.argv[1], sys.argv[2])
  print "Generating message"
  m = DataMessage_pb2.MessageWrapper()
  m.type = DataMessage_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
  m.authentication_message.device_id = "device:test/device1"
  m.authentication_message.user_id = "user:test/user1"
  m.authentication_message.user_key = "dummy"
  print "Sending message"
  client.sendMessageWrapper(m)

  #wait for auth response, then send a data push message
  response = client.receiveMessage()
  if response.authentication_result.result != DataMessage_pb2.AuthenticationResult.SUCCESS:
      print "Authentication failed..."
  
  if(sys.argv[3] == "initial"):
    m = DataMessage_pb2.MessageWrapper()
    m.type = DataMessage_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "content:edu.vanderbilt.isis.ammo.BlueForceTest"
    m.data_message.mime_type = "application/vnd.edu.vu.isis.ammo.battlespace.gcm"
    m.data_message.data = '''
{"uuid":"bumper id","title":"title","description":"description","gcm_type":"UNIT",
"standard_id":"FRIEND","longitude"::-86.7,"latitude":36.1}
'''
    print "Sending bso base information"
    client.sendMessageWrapper(m)

    m.data_message.mime_type = "application/vnd.edu.vu.isis.ammo.battlespace.gcm_anchor"
    m.data_message.data = '''
{"gcm_uuid":"bumper id","longitude":-87.5,"latitude":35.7}
'''
    print "Sending bso anchor information"
    client.sendMessageWrapper(m)
  elif sys.argv[3] == "subscribe": 
    #wait for auth response, then send a data push message
    response = client.receiveMessage()
    if response.authentication_result.result != DataMessage_pb2.AuthenticationResult.SUCCESS:
      print "Authentication failed..."
    m = DataMessage_pb2.MessageWrapper()
    m.type = DataMessage_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = "text/plain"
    print "Sending subscription request..."
    client.sendMessageWrapper(m)
    
  
  #while True:
  #  msg = client.receiveMessage()
  #  print msg
  
  print "Closing socket"
  
