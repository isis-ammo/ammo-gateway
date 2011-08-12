from twisted.protocols import stateful
from twisted.internet import reactor
from twisted.internet.protocol import Factory, Protocol
from twisted.internet.endpoints import TCP4ClientEndpoint

from datetime import datetime
from SecurityManager import SecurityManager

import sys
import struct
import zlib
import time
import threading
import Queue
import uuid

import AmmoMessages_pb2

MAGIC_NUMBER = 0xfeedbeef
DEFAULT_PRIORITY = 0
DEFAULT_RESERVED1 = 0
DEFAULT_RESERVED2 = 0
DEFAULT_RESERVED3 = 0


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
    return (self.receiveHeader, 20) #initial state receives the header
  
  def receiveHeader(self, data):
    (magicNumber, messageSize, priority, error, reserved2, reserved3, checksum, headerChecksum) = struct.unpack("<IIbbbbii", data)
    calculatedHeaderChecksum = zlib.crc32(data[:16])
    if magicNumber != MAGIC_NUMBER:
      print "Invalid magic number!"
    if calculatedHeaderChecksum != headerChecksum:
      print "Header checksum error!"
      print "Expected", headerChecksum
      print "Calculated", calculatedHeaderChecksum
      
    if error != 0 and messageSize == 0 and checksum == 0:
      print "Error received from gateway:"
      print " ", error,
      if error == 1:
        print "Invalid magic number"
      elif error == 2:
        print "Invalid header checksum"
      elif error == 3:
        print "Invalid message checksum"
      elif error == 4:
        print "Message too large"
      else:
        print "Unknown error"
      return (self.receiveHeader, 20)
    else:
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
    
    return (self.receiveHeader, 20)
    
  def sendMessageWrapper(self, msg):
    serializedMsg = msg.SerializeToString()
    messageHeader = struct.pack("<IIbbbbi", MAGIC_NUMBER, len(serializedMsg), DEFAULT_PRIORITY, DEFAULT_RESERVED1, DEFAULT_RESERVED2, DEFAULT_RESERVED3, zlib.crc32(serializedMsg))
    headerChecksum = zlib.crc32(messageHeader)
    messageHeader = messageHeader + struct.pack("i", headerChecksum)
    
    self.transport.write(messageHeader) #little-endian byte order for now
    self.transport.write(serializedMsg);
    
  def connectionMade(self):
    pass
    
  def setOnMessageAvailableCallback(self, callback):
    self._onMessageAvailableCallback = callback
    
class AuthenticationFailure(Exception):
  pass
    
    
class MessageScope:
    GLOBAL = 0
    LOCAL = 1
    
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
  
  def __init__(self, address, port, deviceId, userId, userKey, gatewayId = "MainGateway"):
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
    self._securityManager = SecurityManager(self._deviceId, self._userId, gatewayId, self)
    
  def _gotProtocol(self, p):
    self._protocol = p
    self._onConnect()
    
  def _connect(self):
    factory = Factory()
    factory.protocol = AndroidProtocol
    point = TCP4ClientEndpoint(reactor, self._address, self._port)
    d = point.connect(factory)
    d.addCallback(self._gotProtocol)
    
  def run(self):
    if reactor.running == False:
      self._connect()
      print "Running reactor"
      reactor.run(False) #Argument False tells the reactor that it's not on the
                         #main thread, so it doesn't attempt to register signal
                         #handlers (which doesn't work on other threads)
      print "Reactor stopped"
    else:
      reactor.callFromThread(self._connect)
      print "Reactor is already running...  this background thread will exit."
    
  def _onConnect(self):
    self._protocol.setOnMessageAvailableCallback(self._onMessageAvailable)
    self._securityManager.sendNonce()
    
  def _onMessageAvailable(self, msg):
    if self._authenticated == False:
      if msg.type == AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE:
        self._securityManager.onMessageReceived(msg.authentication_message.type, msg.authentication_message.message, msg.authentication_message.result)
      else:
        print "Got non-auth message during auth process...  dropping."
    else:
      time = datetime.now()
      if self._messageCallback != None:
        self._messageCallback(self, msg)
      
      if self._messageQueueEnabled:
        self._messageQueue.put((msg, time))
    
  def _sendAuthMessage(self, messageType, message):
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.AUTHENTICATION_MESSAGE
    m.authentication_message.type = messageType
    m.authentication_message.message = message
    m.authentication_message.device_id = self._deviceId
    m.authentication_message.user_id = self._userId
    m.authentication_message.user_key = ""
    self._protocol.sendMessageWrapper(m)
    
  def _authenticationFinished(self):
    print "Authentication succeeded."
    
    self._authCondition.acquire()
    self._authenticated = True
    self._authCondition.notifyAll()
    self._authCondition.release()
    
  def dequeueMessage(self):
    '''
    Dequeues a message from the message queue and returns it.  Returns 'none' if
    the queue is empty; otherwise, it returns a pair (message, timeReceived).
    '''
    item = None
    try:
      item = self._messageQueue.get(False) #don't block if queue is empty; raises Empty exception instead
    except Queue.Empty:
      item = None
      pass
    
    return item
    
  def isDataAvailable(self):
    '''
    Checks to see if data is available in the message queue.  Note that, since
    the message queue is filled from a background thread (and could be emptied
    from a background thread), this method returning true/false does not
    necessarily mean that a message will or will not be present when
    dequeueMessage() is called.
    '''
    return not self._messageQueue.empty()
    
  def push(self, uri, mimeType, data, scope = MessageScope.GLOBAL):
    '''
    Sends a push message with the specified URI and MIME type to the gateway.
    '''
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.DATA_MESSAGE
    m.data_message.uri = uri
    m.data_message.mime_type = mimeType
    m.data_message.data = data
    if scope == MessageScope.GLOBAL:
      m.data_message.scope = AmmoMessages_pb2.GLOBAL
    else:
      m.data_message.scope = AmmoMessages_pb2.LOCAL
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def subscribe(self, mimeType, scope = MessageScope.GLOBAL):
    '''
    Subscribes to push data with the specified MIME type.
    
    By default, data received will be placed in the message queue.  The caller
    should periodically call dequeueMessage to receive the push messages that
    it subscribed to.
    '''
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.SUBSCRIBE_MESSAGE
    m.subscribe_message.mime_type = mimeType
    if scope == MessageScope.GLOBAL:
      m.subscribe_message.scope = AmmoMessages_pb2.GLOBAL
    else:
      m.subscribe_message.scope = AmmoMessages_pb2.LOCAL
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def pullRequest(self, mimeType, query, projection, maxResults, startFromCount, liveQuery):
    '''
    Sends a pull request with the specified parameters.  Note that the request
    UID and device ID are automatically set to the correct values (request UID
    is a generated UUID, and device ID is the device ID passed to the
    constructor of this AndroidConnector object).
    '''
    m = AmmoMessages_pb2.MessageWrapper()
    m.type = AmmoMessages_pb2.MessageWrapper.PULL_REQUEST
    m.pull_request.request_uid = uuid.uuid1().hex
    m.pull_request.mime_type = mimeType
    m.pull_request.query = query
    m.pull_request.projection = projection
    m.pull_request.max_results = maxResults
    m.pull_request.start_from_count = startFromCount
    m.pull_request.live_query = liveQuery
    reactor.callFromThread(self._protocol.sendMessageWrapper, m)
    
  def waitForAuthentication(self):
    '''
    Waits for the AndroidConnector to connect to the Android Gateway Plugin, and
    waits for successful authentication.
    
    This method MUST be called after the AndroidConnector's background thread
    is started.  Attempting to call any other member methods of this class
    before authentication is complete has undefined behavior.
    '''
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
