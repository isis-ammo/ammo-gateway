import AmmoMessages_pb2

from M2Crypto import Rand
from M2Crypto import RSA
from M2Crypto import EVP

import hashlib

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
        
      self._serverNonce = message
      self._sendKeyExchange()
      #send Client PhoneAuth
      self._sendPhoneAuth()
      #calculate master_secret
      self._calculateMasterSecret()
      #send Client Finish
      self._sendClientFinish()
      self._state == SecurityManagerState.WAITING_FOR_SERVER_FINISH
    elif messageType == AmmoMessages_pb2.AuthenticationMessage.SERVER_FINISH:
      if self._state != SecurityManagerState.WAITING_FOR_SERVER_FINISH:
        raise InvalidStateException("Received server finish at the wrong time")
      #validate the GatewayFinish
      self._verifyServerFinish()
      #tell the AndroidConnector that everything's done
      self._state == SecurityManagerState.AUTH_COMPLETE
    elif messageType == AmmoMessages_pb2.AuthenticationMessage.STATUS:
      if status == AmmoMessages_pb2.AuthenticationMessage.FAILED:
        raise AuthenticationException("Authentication failed.")
    else:
      raise InvalidStateException("Got an unhandled type of message.")  
      
  def _sendKeyExchange(self):
    self._preMasterSecret = Rand.rand_bytes(PRE_MASTER_SECRET_LENGTH)
    #load the gateway's public key
    gatewayKey = RSA.load_pub_key(self.gatewayId + "_pub.pem")
    self._keyExchange = gatewayKey.public_encrypt(self._preMasterSecret, RSA.pkcs1_padding)
    self._sendMessage(AmmoMessages_pb2.AuthenticationMessage.CLIENT_KEYXCHANGE, self._keyExchange)
    
  def _sendPhoneAuth(self):
    unsignedPhoneAuth = self._keyExchange + self._clientNonce + self._serverNonce
    deviceKey = EVP.load_key(self.deviceId + "_pvt.pem")
    deviceKey.reset_context("sha1")
    deviceKey.sign_init()
    deviceKey.sign_update(unsignedPhoneAuth)
    self._phoneAuth = deviceKey.sign_final()
    self._sendMessage(AmmoMessages_pb2.AuthenticationMessage.CLIENT_PHNAUTH, self._phoneAuth)
    
  def _calculateMasterSecret(self):
    h = hashlib.sha256()
    h.update(self._preMasterSecret + self._clientNonce + self._serverNonce)
    firstStageHash = h.digest()
    
    h2 = hashlib.sha256()
    h2.update(self._preMasterSecret + firstStageHash)
    self._masterSecret = h2.digest()
    
  def _sendClientFinish(self):
    handshake = self_phoneAuth + self._keyExchange + self._clientNonce + self._serverNonce
    content = handshake + self._deviceId + self._masterSecret
    
    h = hashlib.sha256()
    h.update(content)
    firstStageHash = h.digest()
    
    h2 = hashlib.sha256()
    h2.update(masterSecret + content)
    self._clientFinish = h2.digest()
    self._sendMessage(AmmoMessages_pb2.AuthenticationMessage.CLIENT_FINISH, self._clientFinish)
    
  def verifyServerFinish(self, serverFinish):
    handshake = self_phoneAuth + self._keyExchange + self._clientNonce + self._serverNonce
    content = handshake + self._gatewayId + self._masterSecret
    
    h = hashlib.sha256()
    h.update(content)
    firstStageHash = h.digest()
    
    h2 = hashlib.sha256()
    h2.update(masterSecret + content)
    computedServerFinish = h2.digest()
    if serverFinish != computedServerFinish:
      raise AuthenticationException("Server finish doesn't match computed value")
