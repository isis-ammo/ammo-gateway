#include "SerialConnector.h"

#include <sstream>
#include <limits>

#include "SatcomConfigurationManager.h"

void FragmentedMessage::gotMessageFragment(const DataMessage dataHeader, const std::string &data) {
  //Validate message count
  if(dataHeader.count != fragmentsCount) {
    LOG_ERROR("DEFRAGMENTER: Fragment count mismatch; dropping (header = " << dataHeader.count << " stored = " << fragmentsCount << ")");
  } else {
    if(fragments[dataHeader.index]) {
      LOG_ERROR("DEFRAGMENTER: Received duplicate fragment " << dataHeader.index << " of " << fragmentsCount);
    } else {
      fragments[dataHeader.index] = FragmentedMessageDataPtr(new std::string(data)); //fragments is an array of shared_ptrs; this new string
                                                //will be destroyed when fragments is destroyed
      receivedFragmentsCount++;
    }
  }
}

std::string FragmentedMessage::reconstructCompleteMessage() {
  std::ostringstream completeMessage;

  for(FragmentVector::iterator it = fragments.begin(); it != fragments.end(); ++it) {
    completeMessage << *(*it);
  }

  return completeMessage.str();
}

SerialConnector::SerialConnector() :
gatewayConnector(this),
reader(this),
writer(this),
terseDecoder(&gatewayConnector),
closed(true),
eventMutex(),
eventCondition(eventMutex),
lastSignaledEvent(EVENT_NONE),
serialDevice(),
serialConnector(),
tokenTimeout(SatcomConfigurationManager::getInstance().getTokenTimeout()),
initialState(SatcomConfigurationManager::getInstance().getInitialState())
{
  
}

SerialConnector::~SerialConnector() {
  if(!isClosed()) {
    LOG_ERROR("CONNECTOR: SerialReaderThread: Entered destructor before stop()");
    stop();
    this->wait();
  }
}

int SerialConnector::svc() {
  closed = false;

  LOG_DEBUG("CONNECTOR: Connecting to serial port");
  bool status = connect();

  if(status == false) {
   return 1;
  }

  LOG_DEBUG("CONNECTOR: Activating reader and writer threads");
  reader.activate();
  writer.activate();

  SerialConnectorState state = STATE_RECEIVING;
  if(initialState == "sending") {
    state = STATE_SENDING;
  }
  while(!isClosed()) {
    LOG_DEBUG("CONNECTOR: *** SerialConnector loop state: " << state);
    switch(state) {
      case STATE_RECEIVING: {
        //wait for token packet
        SerialConnectorEvent ev = waitForEventSignal();
        if(ev == EVENT_TOKEN_RECEIVED) {
          state = STATE_SENDING;
        } else if(ev == EVENT_RESET_RECEIVED) {
          reset();
          sendResetAck();
          state = STATE_RECEIVING;
        } else if(ev == EVENT_MESSAGE_RECEIVED) {
          //no-op
        } else {
          LOG_WARN("CONNECTOR: Invalid event received in receiving state (" << ev << ")");
        }
        break;
      }
      case STATE_SENDING: {
        sendAckPacket();
        //TODO: send some data packets
        state = STATE_WAITING_FOR_ACK;
        break;
      }
      case STATE_WAITING_FOR_ACK: {
        sendTokenPacket();
        SerialConnectorEvent ev = waitForEventSignal(tokenTimeout);
        if(ev == EVENT_TIMEOUT) {
          //no-op; loop around and resend the token packet
        } else if(ev == EVENT_MESSAGE_RECEIVED) {
          state = STATE_RECEIVING;
        } else if(ev == EVENT_TOKEN_RECEIVED) {
          state = STATE_SENDING;
        } else if(ev == EVENT_RESET_RECEIVED) {
          reset();
          sendResetAck();
          state = STATE_RECEIVING;
        } else {
          LOG_WARN("CONNECTOR: Invalid event received in waiting for ack state (" << ev << ")");
        }
        break;
      }
      default: {
        LOG_ERROR("CONNECTOR: Unknown connector state");
        break;
      }
    }
  }

  reader.stop();
  reader.wait();

  writer.stop();
  writer.wait();

  return 0;
}

bool SerialConnector::connect() {
  const std::string listenPort = SatcomConfigurationManager::getInstance().getListenPort();

  //ACE-based serial initialization code
  int result = serialConnector.connect(serialDevice, ACE_DEV_Addr(listenPort.c_str()));
  if(result == -1) {
    LOG_ERROR("CONNECTOR: Couldn't open serial port " << listenPort << " (" << errno << ": " << strerror(errno) << ")" );
    return false;
  }
  
  ACE_TTY_IO::Serial_Params params;
  params.baudrate = SatcomConfigurationManager::getInstance().getBaudRate();
  params.xonlim = 0;
  params.xofflim = 0;
  params.readmincharacters = 0;
  #ifdef WIN32
  params.readtimeoutmsec = 1;
  #else
  params.readtimeoutmsec = -1; //negative value means infinite timeout
  #endif
  params.paritymode = "NONE";
  params.ctsenb = true;
  params.rtsenb = 1;
  params.xinenb = false;
  params.xoutenb = false;
  params.modem = false;
  params.rcvenb = true;
  params.dsrenb = false;
  params.dtrdisable = false;
  params.databits = 8;
  params.stopbits = 1;

  result = serialDevice.control(ACE_TTY_IO::SETPARAMS, &params);

  if(result == -1) {
    LOG_ERROR("CONNECTOR: Couldn't configure serial port");
    return false;
  }

  return true;
}

void SerialConnector::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    signalEvent(EVENT_CLOSE);
    closed = true;
  } else {
    LOG_ERROR("CONNECTOR: Error acquiring lock in SerialConnector::stop()")
  }
}

bool SerialConnector::isClosed() {
  volatile bool temp;
  {
    ThreadMutexGuard g(closeMutex);
    if(g.locked()) {
      temp = closed;
    } else {
      temp = false;
      LOG_ERROR("CONNECTOR: Error acquiring lock in SerialConnector::isClosed()")
    }
  }
  return temp;
}

void SerialConnector::signalEvent(SerialConnectorEvent ev) {
  ThreadMutexGuard g(eventMutex);
  lastSignaledEvent = ev;
  eventCondition.signal();
}

SerialConnector::SerialConnectorEvent SerialConnector::waitForEventSignal(int timeoutMilliseconds) {
  volatile SerialConnectorEvent result = EVENT_NONE;
  ThreadMutexGuard g(eventMutex);
  if(timeoutMilliseconds != 0) {
    ACE_Time_Value now = ACE_OS::gettimeofday();
    ACE_Time_Value offset;
    offset.set_msec(timeoutMilliseconds);
    ACE_Time_Value timeoutTime = now + offset;
    int status = eventCondition.wait(&timeoutTime);
    if(status == -1 && errno == ETIME) {
      return EVENT_TIMEOUT;
    } else if(status == -1) {
      LOG_ERROR("CONNECTOR: Unknown error while waiting for event signal" << strerror(errno) << " (" << errno << ")");
      return EVENT_NONE;
    } else {
      return lastSignaledEvent;
    }
  } else {
    int status = eventCondition.wait();
    if(status == -1) {
      LOG_ERROR("Unknown error while waiting for event signal" << strerror(errno) << " (" << errno << ")");
      return EVENT_NONE;
    } else {
      return lastSignaledEvent;
    }
  }
}

char SerialConnector::readChar() {
  //LOG_TRACE("In SerialConnector::readChar()");
  unsigned char temp;
  
  ssize_t count = 0;
  while(count == 0 || (count == -1 && errno == ETIME)) {
    ACE_Time_Value timeout(0, 10000);
    //LOG_TRACE("Calling recv_n");
    count = serialDevice.recv_n((void *) &temp, 1, &timeout);
  }

  if ( count == -1 )
  {
    LOG_ERROR("Read returned -1" );
    exit( -1 );
  }
  else if ( count >= 1 )
  {

  }
  else if ( count == 0 )
  {
    LOG_ERROR("Read returned 0" );
    exit( -1 );
  }

  //LOG_TRACE("Serial Received " << std::hex << (int) temp);
  return temp;
}

bool SerialConnector::writeMessageFragment(const std::string &message) {
  std::ostringstream hexOutput;

  for(size_t i = 0; i < message.length(); i++) {
    hexOutput << std::hex << ((unsigned int) message[i] & 0xff) << " ";
  }

  LOG_TRACE("Sending message: " << hexOutput.str());

  ssize_t bytesWritten = serialDevice.send_n(message.data(), message.length());

  if(bytesWritten == -1) {
    LOG_ERROR("Write returned -1");
    exit(-1);
  } else if(bytesWritten != message.length()) {
    LOG_WARN("Didn't send entire message");
  }

  return bytesWritten;
}

void SerialConnector::receivedMessageFragment(const DataMessage dataHeader, const uint8_t shouldAck, const uint8_t dataType, const std::string &data) {
  LOG_DEBUG("Received message fragment");
  LOG_DEBUG("   Seq Number: " << dataHeader.sequenceNumber);
  LOG_DEBUG("        Count: " << dataHeader.count);
  LOG_DEBUG("        Index: " << dataHeader.index);
  LOG_DEBUG("   Should Ack: " << (int) shouldAck);
  LOG_DEBUG("    Data Type: " << (int) dataType);

  uint16_t firstMessageSequenceNumber = dataHeader.sequenceNumber - dataHeader.count;

  IncompleteMessageMap::iterator messageIt = incompleteMessages.find(firstMessageSequenceNumber);

  if(messageIt == incompleteMessages.end()) {
    FragmentedMessage newMessage(firstMessageSequenceNumber, dataHeader.count, dataType);
    std::pair<IncompleteMessageMap::iterator, bool> result = incompleteMessages.insert(IncompleteMessageMap::value_type(firstMessageSequenceNumber, newMessage));

    if(result.second == true) {
      messageIt = result.first;
    } else {
      LOG_ERROR("receivedMessageFragment: insertion failed");
    }
  }

  messageIt->second.gotMessageFragment(dataHeader, data);

  if(messageIt->second.isMessageComplete()) {
    std::string completeMessage = messageIt->second.reconstructCompleteMessage();
    processMessage(dataType, completeMessage);
    incompleteMessages.erase(messageIt);
  }

  if(shouldAck == 1) {
    ThreadMutexGuard g(sequenceNumbersToAckMutex);
    sequenceNumbersToAck.push(dataHeader.sequenceNumber);
  }
}

void SerialConnector::receivedAckPacket(const bool isToken, const std::vector<uint16_t> &acks) {
  if(isToken) {
    signalEvent(EVENT_TOKEN_RECEIVED);
  } else {
    signalEvent(EVENT_MESSAGE_RECEIVED);
    LOG_WARN("Received ack; not yet implemented");
  }
}

void SerialConnector::receivedReset() {
  signalEvent(EVENT_RESET_RECEIVED);
}

void SerialConnector::processMessage(const uint8_t dataType, const std::string &message) {
  LOG_DEBUG("Received data message of type " << dataType);
  terseDecoder.processMessage(dataType, message);
}

SerialConnector::SequenceNumberQueue SerialConnector::getSequenceNumbersToAck() {
  SequenceNumberQueue result;

  ThreadMutexGuard g(sequenceNumbersToAckMutex);

  size_t count = 0;
  size_t sequenceNumberLimit = (std::numeric_limits<uint16_t>::max()) / 2;
  while(!sequenceNumbersToAck.empty() && count < sequenceNumberLimit) {
    result.push(sequenceNumbersToAck.front());
    sequenceNumbersToAck.pop();
    count++;
  }

  return result;
}

void SerialConnector::reset() {
  incompleteMessages.clear();

  {
    ThreadMutexGuard g(sequenceNumbersToAckMutex);
    while(!sequenceNumbersToAck.empty()) {
      sequenceNumbersToAck.pop();
    }
  }
}

/*
* Message Type bitfield structure:
* abcd dddd
* 
* a: Packet type (0 = data, 1 = ack or token)
* b: Reset (0 = normal, 1 = reset)
* c: should ack (0 = don't ack; 1 = do ack)
* d dddd: Datatype-specific identifier (set to 0 for ack/token packets)
*/

void SerialConnector::sendResetAck() {
  SatcomHeader header;
  header.magicNumber = MAGIC_NUMBER;
  header.size = 1;
  header.reserved = 0;

  uint8_t messageTypeByte = 0xc0; //1100 0000

  header.payloadChecksum = ACE::crc32(&messageTypeByte, sizeof(messageTypeByte));
  header.headerChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string);
  messageToSend->reserve(sizeof(header) + sizeof(messageTypeByte));
  messageToSend->append(reinterpret_cast<char *>(&header), sizeof(header));
  messageToSend->append(reinterpret_cast<char *>(&messageTypeByte), sizeof(messageTypeByte));

  writer.queueMessage(messageToSend);
}

void SerialConnector::sendAckPacket() {
  SequenceNumberQueue sequenceNumbers = getSequenceNumbersToAck();

  SatcomHeader header;
  header.magicNumber = MAGIC_NUMBER;
  header.reserved = 0;

  std::ostringstream ackPacketDataStream; //first byte is message type; second two bytes are ack info
  appendUInt8(ackPacketDataStream, 0x80); //1000 0000; ack or token packet
  appendUInt16(ackPacketDataStream, sequenceNumbers.size() & 0xefff); //mask off high byte; if it's set, this is a token (which it isn't)

  while(!sequenceNumbers.empty()) {
    appendUInt16(ackPacketDataStream, sequenceNumbers.front());
    sequenceNumbers.pop();
  }

  std::string ackPacketData = ackPacketDataStream.str();

  header.size = ackPacketData.length();
  header.payloadChecksum = ACE::crc32(ackPacketData.data(), ackPacketData.length());
  header.headerChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string);
  messageToSend->reserve(sizeof(header) + ackPacketData.length());
  messageToSend->append(reinterpret_cast<char *>(&header), sizeof(header));
  messageToSend->append(ackPacketData);

  writer.queueMessage(messageToSend);
}

void SerialConnector::sendTokenPacket() {
  SequenceNumberQueue sequenceNumbers = getSequenceNumbersToAck();

  SatcomHeader header;
  header.magicNumber = MAGIC_NUMBER;
  header.reserved = 0;

  std::ostringstream ackPacketDataStream; //first byte is message type; second two bytes are ack info
  appendUInt8(ackPacketDataStream, 0x80); //1000 0000; ack or token packet
  appendUInt16(ackPacketDataStream, 0x8000); //1000 0000 0000 0000; high byte set indicates that this is a token packet; no acks included

  std::string ackPacketData = ackPacketDataStream.str();

  header.size = ackPacketData.length();
  header.payloadChecksum = static_cast<uint16_t>(ACE::crc32(ackPacketData.data(), ackPacketData.length()) & 0x0000ffff);
  header.headerChecksum = static_cast<uint16_t>(ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum)) & 0x0000ffff);

  LOG_TRACE("Payload checksum " << std::hex << header.payloadChecksum);
  LOG_TRACE("Header checksum " << std::hex << header.headerChecksum);

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string);
  messageToSend->reserve(sizeof(header) + ackPacketData.length());
  messageToSend->append(reinterpret_cast<char *>(&header), sizeof(header));
  messageToSend->append(ackPacketData);

  LOG_TRACE("Sending token packet");
  writer.queueMessage(messageToSend);
}

void SerialConnector::appendUInt8(std::ostream &stream, const uint8_t val) {
  stream.write(reinterpret_cast<const char *>(&val), sizeof(val));
}


void SerialConnector::appendUInt16(std::ostream &stream, const uint16_t val) {
  stream.write(reinterpret_cast<const char *>(&val), sizeof(val));
}

//GatewayConnectorDelegate methods
void SerialConnector::onConnect(ammo::gateway::GatewayConnector *sender) {
  //do nothing
}

void SerialConnector::onDisconnect(ammo::gateway::GatewayConnector *sender) {
  //do nothing
}