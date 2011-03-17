from twisted.protocols import stateful
from twisted.internet import reactor
from twisted.internet.protocol import Factory, Protocol
from twisted.internet.endpoints import TCP4ClientEndpoint

from datetime import datetime

import sys
import struct
import zlib
import time
import threading
import Queue
import uuid

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
  _onMessageAvailableCallback = None
  
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
    
    if self._onMessageAvailableCallback != None:
      self._onMessageAvailableCallback(msg)
    
    return (self.receiveHeader, 8)
    
  def sendMessageWrapper(self, msg):
    serializedMsg = msg.SerializeToString()
    self.transport.write(struct.pack("<Ii", len(serializedMsg), zlib.crc32(serializedMsg))) #little-endian byte order for now
    self.transport.write(serializedMsg);
    
  def connectionMade(self):
    print "connectionMade"
    
  def setOnMessageAvailableCallback(self, callback):
    self._onMessageAvailableCallback = callback
    
class AuthenticationFailure(Exception):
  pass
    
class AndroidConnector(threading.Thread):
  _address = ""
  _port = 0
  _deviceId = ""
  _userId = ""
  _userKey = ""
  
  _protocol = None
  
  _authenticated = False
  _authCondition = None
  
  _messageQueueEnabled = True
  _messageQueue = None
  _messageCallback = None
  
  def __init__(self, address, port, deviceId, userId, userKey):
    threading.Thread.__init__(self)
    self._address = address
    self._port = port
    self._deviceId = deviceId
    self._userId = userId
    self._userKey = userKey
    
    self._authenticated = False
    self._authCondition = threading.Condition()
    
    self._messageQueueEnabled = True
    self._messageQueue = Queue.Queue()
    self._messageCallback = None
    
  def _gotProtocol(self, p):
    print "gotProtocol"
    self._protocol = p
    self._onConnect()
    
  def run(self):
    factory = Factory()
    factory.protocol = AndroidProtocol
    point = TCP4ClientEndpoint(reactor, self._address, self._port)
    d = point.connect(factory)
    d.addCallback(self._gotProtocol)
    print "Running reactor"
    reactor.run(False)
    print "Reactor stopped"
    
  def _onConnect(self):
    self._protocol.setOnMessageAvailableCallback(self._onMessageAvailable)
    self._sendAuthMessage()
    
  def _onMessageAvailable(self, msg):
    if self._authenticated == False:
      if msg.type == AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_RESULT:
        if msg.authentication_result.result == AmmoMessages_pb2.AuthenticationResult.SUCCESS:
          print "Authentication succeeded."
          self._authCondition.acquire()
          self._authenticated = True
          self._authCondition.notifyAll()
          self._authCondition.release()
        else:
          print "Authentication failed."
          raise AuthenticationFailure("Auth failed: " + msg.authentication_result.message)
    
    time = datetime.now()
    if self._messageCallback != None:
      self._messageCallback(self, msg)
    
    if self._messageQueueEnabled:
      self._messageQueue.put((msg, time))
    
  def _sendAuthMessage(self):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
    m.authentication_message.device_id = self._deviceId
    m.authentication_message.user_id = self._userId
    m.authentication_message.user_key = self._userKey
    print "Sending auth message"
    self._protocol.sendMessageWrapper(m)
    
    
  #Dequeues a message from the message queue and returns it.  Returns 'none' if
  #the queue is empty; otherwise, it returns a pair (message, timeReceived)
  def dequeueMessage(self):
    item = None
    try:
      item = self._messageQueue.get(False) #don't block if queue is empty; raises Empty exception instead
    except Queue.Empty:
      item = None
      pass
    
    return item
    
  def isDataAvailable(self):
    return not self._messageQueue.empty()
    
  def push(self, uri, mimeType, data):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = uri
    m.data_message.mime_type = mimeType
    m.data_message.data = data
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def subscribe(self, mimeType):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = mimeType
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def pullRequest(mimeType, query, projection, maxResults, startFromCount, liveQuery):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
    m.pull_request.request_uid = uuid.uuid1().hex
    m.pull_request.plugin_id = self._deviceId
    m.pull_request.mime_type = mimeType
    m.pull_request.query = query
    m.pull_request.projection = projection
    m.pull_request.max_results = maxResults
    m.pull_request.start_from_count = startFromCount
    m.pull_request.live_query = liveQuery
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def waitForAuthentication(self):
    self._authCondition.acquire()
    if self._authenticated == False:
      self._authCondition.wait()
    self._authCondition.release()
    
  def registerMessageCallback(self, callback):
    '''
    Registers a callback method to be called when a message is received.  Note
    that this callback is called on the *event loop's* thread--  which may not
    be the thread where the caller (of this method) is running.  The caller is
    expected to handle any synchronization issues which might result.
    
    Also note that registering this callback does not disable the message queue--
    the consumer of AndroidConnector will want to either drain this queue or 
    disable it with AndroidConnector.setMessageQueueEnabled(False) to avoid 
    memory leaks.
    '''
    self._messageCallback = callback
    
  def setMessageQueueEnabled(self, enabled):
    '''
    Enables or disables the message queue.  The message queue is enabled by
    default; you might want to disable it if, for example, you only want to
    print messages as they are received in a callback.
    
    setMessageQueueEnabled(false) should almost always be used in conjunction
    with registerMessageCallback, or you will lose any messages received while
    the message queue is disabled.
    '''
    self._messageQueueEnabled = enabled
    
# Main method for this class (not run when it's imported).
# This is a usage example for the AndroidConnector--  it subscribes to a data
# type, then prints out any data that it receives with that type.
if __name__ == "__main__":
  print "Android Gateway Tester"
  connector = AndroidConnector("localhost", 32869, "device:test/pythonTestDriver1", "user:user/testPythonUser1", "")
  
  try:
    connector.start()
    
    connector.waitForAuthentication()
    
    print "Subscribing to type text/plain"
    connector.subscribe("text/plain")
  
    while True:
      while(connector.isDataAvailable()):
        result = connector.dequeueMessage()
        if(result != None):
          (msg, receivedTime) = result
          print "Message received at:", receivedTime
          print msg
      time.sleep(0.5)
      
  except KeyboardInterrupt:
    print "Got ^C...  Closing"
    reactor.callFromThread(reactor.stop)
    # re-raising the exception so we get a traceback (useful for debugging,
    # occasionally).  Real "applications"/testdrivers shouldn't do this.
    raise 
  except:
    print "Unexpected error...  dying."
    reactor.callFromThread(reactor.stop)
    raise
