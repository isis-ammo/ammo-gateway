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

if __name__ == "__main__":
  print "Android Gateway Tester"
  if len(sys.argv) != 3:
    print "Usage:", sys.argv[0], "host port"
  
  print "Creating client"
  client = GatewayTestClient(sys.argv[1], sys.argv[2])
  print "Generating message"
  m = DataMessage_pb2.MessageWrapper()
  m.type = DataMessage_pb2.MessageWrapper.DATA_MESSAGE
  #m.data_message = DataMessage_pb2.DataMessage()
  m.data_message.uri = "urn:edu.vanderbilt.isis.ammmo.test.string"
  m.data_message.mime_type = "text/plain"
  m.data_message.data = "Hello World"
  print "Sending message"
  client.sendMessageWrapper(m)
  print "Closing socket"
  
