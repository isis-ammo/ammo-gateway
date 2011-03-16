from twisted.protocols import stateful
from twisted.internet import reactor
from twisted.internet.protocol import Factory, Protocol
from twisted.internet.endpoints import TCP4ClientEndpoint

import sys
import struct
import zlib
import time
import threading

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
  _onConnectCallback = None
  
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
    print "connectionMade"
    
  def setOnConnectCallback(self, callback):
    print "setOnConnectCallback"
    self._onConnectCallback = callback
    
    
class AndroidConnector(threading.Thread):
  _address = ""
  _port = 0
  _protocol = None
  
  _authenticated = False
  _authCondition = None
  
  def __init__(self, address, port):
    threading.Thread.__init__(self)
    self._address = address
    self._port = port
    self._authenticated = False
    self._authCondition = threading.Condition()
    
  def gotProtocol(self, p):
    print "gotProtocol"
    self._protocol = p
    p.setOnConnectCallback(self.onConnect)
    self.onConnect()
    
  def run(self):
    factory = Factory()
    factory.protocol = AndroidProtocol
    point = TCP4ClientEndpoint(reactor, self._address, self._port)
    d = point.connect(factory)
    d.addCallback(self.gotProtocol)
    print "Running reactor"
    reactor.run(False)
    print "Reactor stopped"
    
  def onConnect(self):
    self.sendAuthMessage()
    
  def sendAuthMessage(self):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
    m.authentication_message.device_id = "device:test/device1"
    m.authentication_message.user_id = "user:test/user1"
    m.authentication_message.user_key = "dummy"
    print "Sending auth message"
    self._protocol.sendMessageWrapper(m)
    self._authCondition.acquire()
    self._authenticated = True
    self._authCondition.notifyAll()
    self._authCondition.release()
    
  def push(self, uri, mimeType, data):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = uri
    m.data_message.mime_type = mimeType
    m.data_message.data = data
    self._protocol.sendMessageWrapper(m)
    
  def subscribe(self, mimeType):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = mimeType
    self._protocol.sendMessageWrapper(m)
    
  def waitForAuthentication(self):
    self._authCondition.acquire()
    if self._authenticated == False:
      self._authCondition.wait()
    self._authCondition.release()
    
if __name__ == "__main__":
  print "Android Gateway Tester"
  connector = AndroidConnector("localhost", 32869)
  
  try:
    connector.start()
    
    connector.waitForAuthentication()
    
    print "Subscribing to type text/plain"
    connector.subscribe("text/plain")
  
    while True:
      time.sleep(0.5)
  #except KeyboardInterrupt:
  #  print "Closing"
  #  reactor.callFromThread(reactor.stop)
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
