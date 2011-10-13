#!/usr/bin/env python

#Test driver for Android gateway plugin to test deserialization of messages.

import sys
import socket
import struct
import zlib
import time

import AmmoMessages_pb2

class PullRequestTestClient:
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

if __name__ == "__main__":
  print "Pull Request Tester"
  if len(sys.argv) != 3:
    print "Usage:", sys.argv[0], "host port"
    exit(-1)
  
  print "Creating client"
  client = PullRequestTestClient(sys.argv[1], sys.argv[2])
  print "Generating message"
  m = AmmoMessages_pb2.MessageWrapper()
  m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
  m.authentication_message.device_id = "device:test/device1"
  m.authentication_message.user_id = "user:test/user1"
  m.authentication_message.user_key = "dummy"
  print "Sending message"
  client.sendMessageWrapper(m)
  
  #wait for auth response, then send a data push message
  response = client.receiveMessage()
  if response.authentication_result.result != AmmoMessages_pb2.AuthenticationResult.SUCCESS:
    print "Authentication failed..."
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
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = "type:edu.vanderbilt.isis.ammo.Test"
    m.data_message.mime_type = "ammo/edu.vu.isis.ammo.sms.message"
    m.data_message.data = "{ \"sms_uri\" : \"type:edu.vanderbilt.isis.ammo.Test\", "
    m.data_message.data += "\"sender\" : \"sgt_kill_em_all@151st.mil\", "
    m.data_message.data += "\"recipient\" : \"soldier_"
    m.data_message.data += str(n)
    m.data_message.data += "\", "
    m.data_message.data += "\"thread\" : 14, "
    m.data_message.data += "\"payload\" : \"hey soldier_"
    m.data_message.data += str(n)
    m.data_message.data += ", meet me at the PX\", "
    m.data_message.data += "\"createdDate\" : "
    m.data_message.data += str(start)
    m.data_message.data += ", "
    m.data_message.data += "\"modifiedDate\" : "
    m.data_message.data += str(start)
    m.data_message.data += "}"
    output_msg = "Sending data message "
    output_msg += str(n)
    print output_msg
    client.sendMessageWrapper(m)
    msg = client.receiveMessage()
    print msg
    n += 1
	
  # Send a single pull request.
  m = AmmoMessages_pb2.MessageWrapper()
  m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
  m.pull_request.mime_type = "ammo/edu.vu.isis.ammo.sms.message"
  m.pull_request.request_uid = "PRT_pull_request"
  # max_results is unlimited, but the use of the start time string
  # will limit responses to the 5 pushes above, no matter how many
  # times this test has been run previously.
  m.pull_request.max_results = 0
  m.pull_request.query = ",,"
  m.pull_request.query += str(start)
  m.pull_request.query += ",,"
  
  # This string will filter the responses to a single message.
  m.pull_request.projection = ",,soldier_2,,,,,,,"
  
  print "Sending pull request..."
  client.sendMessageWrapper(m)
  
  # Receive and print out all the pullResponse() callbacks.
  # We should get only one response, the text message sent to soldier_2.
  while True:
    msg = client.receiveMessage()
    print msg
    

  print "Closing socket"
  
