#!/usr/bin/env python

#Test driver for Android gateway plugin to test deserialization of messages.

import sys
import socket
import struct
import zlib

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
  print "Android Gateway Aterrasys Message"
  if len(sys.argv) != 4:
    print "Usage:", sys.argv[0], "host port message-type"
    print '''
  where message-type is one of:"
    authenticate : always run, this a dummy actually anything would work.
    subscribe : subscribe to type:urn:aterrasys.com:/api/chat/msg
    pull : pull type:urn:aterrasys.com:/api/rtc/people/list
    push : send a data message of topic type:urn:aterrasys.com:/api/chat/invite
    pushLoc : send a data message of topic type:urn:aterrasys.com:/api/location/post
    pushLocs : send a data message of topic type:urn:aterrasys.com:/api/locations/post
'''
    exit(-1)
  
  print "Creating client"
  client = GatewayTestClient(sys.argv[1], sys.argv[2])
  print "Generating message"
  m = AmmoMessages_pb2.MessageWrapper()
  m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
  m.authentication_message.device_id = "device:test/ats"
  m.authentication_message.user_id = "user:test/atsPlugin"
  m.authentication_message.user_key = "secret"
  print "Sending message"
  client.sendMessageWrapper(m)

  print "Waiting for Authentication Response"
    #wait for auth response, then send a data push message
  response = client.receiveMessage()
  if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
    print "Authentication failed..."
  
  if(sys.argv[3] == "push"):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "type:urn:aterrasys.com:/api/chat/invite"
    m.data_message.mime_type = "urn:aterrasys.com:/api/chat/invite"
    m.data_message.data = "This is a message intended for the Aterrasys service."
    print "Sending data message", m.data_message.data
    client.sendMessageWrapper(m)

  elif(sys.argv[3] == "pushLoc"):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "type:urn:aterrasys.com:/api/location/post/"
    m.data_message.mime_type = "urn:aterrasys.com:/api/location/post/"
    m.data_message.data = '{"lat":2000000,"lon":2000000}'
    print "Sending data message", m.data_message.data
    client.sendMessageWrapper(m)

  elif(sys.argv[3] == "pushLocs"):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "type:urn:aterrasys.com:/api/locations/post/"
    m.data_message.mime_type = "urn:aterrasys.com:/api/locations/post/"
    m.data_message.data = 'json=[{"user":"sbasu","lat":2000000,"lon":2000000},{"user":"dutch","lat":2000000,"lon":2000000}]'
    print "Sending data message", m.data_message.data
    client.sendMessageWrapper(m)

  elif sys.argv[3] == "subscribe": 
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = "urn:aterrasys.com:/api/rtc/people/add"
    print "Sending subscription request..."
    client.sendMessageWrapper(m)

  elif sys.argv[3] == "pull":
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
    m.pull_request.request_uid = "ats-people-req-1"
    m.pull_request.plugin_id = "aterrasys-plugin-test"
    #m.pull_request.query = "x-Callsign = Hawk*"
    m.pull_request.query = """{ "selection":"*" }"""
    m.pull_request.mime_type = "urn:aterrasys.com:/api/rtc/people/list"
    print "Sending pull request..."
    client.sendMessageWrapper(m)

  while True:
    msg = client.receiveMessage()
    print msg
  
  print "Closing socket"
  
