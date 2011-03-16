from twisted.protocols import stateful
from twisted.internet import reactor
from twisted.internet.protocol import Factory, Protocol
from twisted.internet.endpoints import TCP4ClientEndpoint

import sys
import struct
import zlib
import time

import AmmoMessages_pb2

class AndroidProtocol(stateful.StatefulProtocol):
  '''
  This class implements the stateful Android <-> Gateway protocol.  It contains
  an 8-byte header with messageSize and a checksum, then a protobuf
  MessageWrapper object (of length messageSize).
  
  We use Twisted's StatefulProtocol to implement this protocol--  states are
  composed of a callback function and a size; Twisted calls the callback
  function when <size> data has been received.  The callback functions return
  the next state.
  '''
  _messageSize = 0
  _checksum = 0
  
  def getInitialState(self):
    return (self.receiveHeader, 8) #initial state receives the header
  
  def receiveHeader(self, data):
    (messageSize, checksum) = struct.unpack("<Ii", data)
    self._messageSize = messageSize
    self._checksum = checksum
    
    return (self.receiveData, self._messageSize)
    
  def receiveData(self, data):
    calculatedChecksum = zlib.crc32(data)
    if calculatedChecksum != self._checksum:
      print "Checksum error!"
    msg = AmmoMessages_pb2.MessageWrapper()
    msg.ParseFromString(data)
    print msg
    return (self.receiveHeader, 8)
    
  def sendMessageWrapper(self, msg):
    serializedMsg = msg.SerializeToString()
    self.transport.write(struct.pack("<Ii", len(serializedMsg), zlib.crc32(serializedMsg))) #little-endian byte order for now
    self.transport.write(serializedMsg);
    
  def connectionMade(self):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
    m.authentication_message.device_id = "device:test/device1"
    m.authentication_message.user_id = "user:test/user1"
    m.authentication_message.user_key = "dummy"
    print "Sending auth message"
    self.sendMessageWrapper(m)
    
if __name__ == "__main__":
  print "Android Gateway Tester"
  factory = Factory()
  factory.protocol = AndroidProtocol
  point = TCP4ClientEndpoint(reactor, "localhost", 32869)
  d = point.connect(factory)
  reactor.run()
