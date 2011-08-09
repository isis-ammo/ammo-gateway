import AmmoMessages_pb2

from M2Crypto import Rand

NONCE_LENGTH = 20
PRE_MASTER_SECRET_LENGTH = 48

class InvalidStateException(Exception):
  pass

class AuthenticationException(Exception):
  pass

class SecurityManagerState:
  CLIENT_NONCE_NOT_SENT = 1
  WAITING_FOR_SERVER_NONCE = 2
  WAITING_FOR_SERVER_FINISH = 3
  AUTH_COMPLETE = 4

class SecurityManager:
  def __init__(self, deviceId, userId, gatewayId, androidConnector):
    self._deviceId = deviceId
    self._gatewayId = gatewayId
    self._androidConnector = androidConnector
    self._state = SecurityManagerState.SERVER_NONCE_NOT_SENT
    
  def _sendMessage(self, messageType, message):
    self._androidConnector._sendAuthMessage(messageType, message)
    
  def sendNonce(self):
    self._clientNonce = Rand.rand_bytes(NONCE_LENGTH)
    print "Sending client nonce"
    self._sendMessage(AmmoMessages_pb2.AuthenticationMessage.CLIENT_NONCE, self._clientNonce)
    self._state = SecurityManagerState.WAITING_FOR_SERVER_NONCE
    
  def onMessageReceived(self, messageType, message, status):
    if self._state == SecurityManagerState.CLIENT_NONCE_NOT_SENT:
      raise InvalidStateException("Auth message received before client nonce was sent")
    if self._state == SecurityManagerState.AUTH_COMPLETE:
      raise InvalidStateException("Auth message received after auth was finished")
    elif messageType == AmmoMessages_pb2.AuthenticationMessage.SERVER_NONCE:
      if self._state != SecurityManagerState.WAITING_FOR_SERVER_NONCE:
        raise InvalidStateException("Received server nonce at the wrong time")
      #do some stuff here:
      #store the server nonce
      
      #send Client KeyExchange
      #send Client PhoneAuth
      #calculate master_secret
      #send Client Finish
      self._state == SecurityManagerState.WAITING_FOR_SERVER_FINISH
    elif messageType == AmmoMessages_pb2.AuthenticationMessage.SERVER_FINISH:
      if self._state != SecurityManagerState.WAITING_FOR_SERVER_FINISH:
        raise InvalidStateException("Received server finish at the wrong time")
      #validate the GatewayFinish
      #tell the AndroidConnector that everything's done
      self._state == SecurityManagerState.AUTH_COMPLETE
    elif messageType == AmmoMessages_pb2.AuthenticationMessage.STATUS:
      if status == AmmoMessages_pb2.AuthenticationMessage.FAILED:
        raise AuthenticationException("Authentication failed.")
    else:
      raise InvalidStateException("Got an unhandled type of message.")  
