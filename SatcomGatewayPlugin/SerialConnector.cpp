#include "SerialConnector.h"

#include <sstream>
#include <limits>

#include <protocol/AmmoMessages.pb.h>

#include "SatcomConfigurationManager.h"
#include "BinaryOutputStream.h"
#include "TerseEncoder.h"

const char *MIME_TYPE_TRANSFER_STATUS = "ammo/edu.vu.isis.ammo.satcom.transfer_status";

DataMessageFragment::DataMessageFragment() : 
sequenceNumber(0),
index(0),
count(0),
shouldAck(false),
messageData()
{
  //do nothing
}

/*
* Message Type bitfield structure:
* abcd eeee
* 
* a: Packet type (0 = data, 1 = ack or token)
* b: Reset (0 = normal, 1 = reset)
* c: should ack (0 = don't ack; 1 = do ack)
* d: sender ID (0 = gateway; 1 = device); messages from gateway should be ignored (over these radios, everything we send is echoed back)
* eeee: Unused (set to 0)
*/

DataMessageFragment::MessageDataPtr DataMessageFragment::serializeFragment() {
  if(messageData) {
    BinaryOutputStream stream;

    uint8_t messageInfoByte = 0x10 | ((((uint8_t) shouldAck) & 0x01) << 5);
    stream.appendUInt8(messageInfoByte);

    DataMessage messageHeader;
    messageHeader.sequenceNumber = sequenceNumber;
    messageHeader.index = index;
    messageHeader.count = count;

    stream.appendBytes(reinterpret_cast<char *>(&messageHeader), sizeof(messageHeader));
    stream.appendBytes(*messageData);

    return MessageDataPtr(new std::string(stream.getString()));
  } else {
    LOG_WARN("Attempted to serialize a DataMessageFragment with no data");
    return MessageDataPtr();
  }
}

void FragmentedMessage::gotMessageFragment(const DataMessage dataHeader, const std::string &data) {
  //Validate message count
  if(dataHeader.count != fragmentsCount) {
    LOG_ERROR("DEFRAGMENTER: Fragment count mismatch; dropping (header = " << dataHeader.count << " stored = " << fragmentsCount << ")");
  } else {
    if(dataHeader.index < fragmentsCount) {
      if(fragments[dataHeader.index]) {
        LOG_ERROR("DEFRAGMENTER: Received duplicate fragment " << dataHeader.index << " of " << fragmentsCount);
      } else {
        std::ostringstream loggingStream;

        for(std::string::const_iterator it = data.begin(); it != data.end(); it++) {
          loggingStream << std::hex << (((unsigned int) *it) & 0xff) << " ";
        }

        LOG_TRACE("Fragment " << dataHeader.index << ": " << loggingStream.str());
        fragments[dataHeader.index] = FragmentedMessageDataPtr(new std::string(data)); //fragments is an array of shared_ptrs; this new string
                                                  //will be destroyed when fragments is destroyed
        receivedFragmentsCount++;
      }
    } else {
      LOG_ERROR("DEFRAGMENTER: Received invalid index (i=" << dataHeader.index << ", c=" << fragmentsCount << ")");
    }
  }
  LOG_TRACE("DEFRAGMENTER: Have received total " << receivedFragmentsCount << " of " << fragmentsCount);
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
nextSequenceNumber(0),
tokenTimeout(SatcomConfigurationManager::getInstance().getTokenTimeout()),
initialState(SatcomConfigurationManager::getInstance().getInitialState()),
fragmentSize(1024), //TODO: make configurable
fragmentsToSendPerCycle(3)
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
        //sendDataFragments(); //FIXME: turned off for build to Brad
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

  LOG_DEBUG("Stopping reader thread");
  reader.stop();
  LOG_DEBUG("Waiting for reader thread to terminate...");
  reader.wait();

  LOG_DEBUG("Stopping writer thread");
  writer.stop();
  LOG_DEBUG("Waiting for writer thread to terminate...");
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

bool SerialConnector::writeMessageFragment(const std::string &message) {
  SatcomHeader header;
  header.magicNumber = MAGIC_NUMBER;
  header.size = message.length();
  header.reserved = 0;
  header.payloadChecksum = ACE::crc32(message.data(), message.length());
  header.headerChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  std::string messageToSend;
  messageToSend.reserve(sizeof(header) + message.length());
  messageToSend.append(reinterpret_cast<char *>(&header), sizeof(header));
  messageToSend.append(message);

  std::ostringstream hexOutput;

  for(size_t i = 0; i < messageToSend.length(); i++) {
    hexOutput << std::hex << ((unsigned int) messageToSend[i] & 0xff) << " ";
  }

  LOG_TRACE("Sending message: " << hexOutput.str());

  ssize_t bytesWritten = serialDevice.send_n(messageToSend.data(), messageToSend.length());

  if(bytesWritten == -1) {
    LOG_ERROR("Write returned -1");
    exit(-1);
  } else if(bytesWritten != messageToSend.length()) {
    LOG_WARN("Didn't send entire message");
  }

  return bytesWritten;
}

void SerialConnector::receivedMessageFragment(const DataMessage dataHeader, const uint8_t shouldAck, const uint8_t dataType, const std::string &data) {
  signalEvent(EVENT_MESSAGE_RECEIVED);

  LOG_DEBUG("Received message fragment");
  LOG_DEBUG("   Seq Number: " << dataHeader.sequenceNumber);
  LOG_DEBUG("        Count: " << dataHeader.count);
  LOG_DEBUG("        Index: " << dataHeader.index);
  LOG_DEBUG("   Should Ack: " << (int) shouldAck);
  LOG_DEBUG("    Data Type: " << (int) dataType);

  uint16_t firstMessageSequenceNumber = dataHeader.sequenceNumber - dataHeader.index;

  IncompleteMessageMap::iterator messageIt = incompleteMessages.find(firstMessageSequenceNumber);

  if(messageIt == incompleteMessages.end()) {
    LOG_TRACE("Got first fragment of a new message, creating new FragmentedMessage");
    FragmentedMessage newMessage(firstMessageSequenceNumber, dataHeader.count, dataType);
    std::pair<IncompleteMessageMap::iterator, bool> result = incompleteMessages.insert(IncompleteMessageMap::value_type(firstMessageSequenceNumber, newMessage));

    if(result.second == true) {
      messageIt = result.first;
    } else {
      LOG_ERROR("receivedMessageFragment: insertion failed");
    }
  }

  messageIt->second.gotMessageFragment(dataHeader, data);

  if(messageIt->second.getFragmentsCount() != 1) {
    std::ostringstream transferStatusPushData;
    transferStatusPushData << "{\"index\":\"" << messageIt->second.getStartingSequenceNumber();
    transferStatusPushData << "\",\"received\":\"" << messageIt->second.getReceivedFragmentsCount();
    transferStatusPushData << "\",\"total\":\"" << messageIt->second.getFragmentsCount() << "\"}";

    ammo::gateway::PushData transferStatusPush;
    transferStatusPush.uri = "satcom-status-update";
    transferStatusPush.encoding = "json";
    transferStatusPush.mimeType = MIME_TYPE_TRANSFER_STATUS;
    transferStatusPush.ackThresholds.deviceDelivered = false;
    transferStatusPush.ackThresholds.pluginDelivered = false;
    transferStatusPush.scope = ammo::gateway::SCOPE_LOCAL;
    transferStatusPush.data = transferStatusPushData.str();

    gatewayConnector.pushData(transferStatusPush);

  }

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

void SerialConnector::enqueueDataMessage(const std::string &message, const bool shouldAck) {
  ThreadMutexGuard g(sendDataMapMutex);
  unsigned int numberOfFragments = message.length() / fragmentSize;
  if(message.length() % fragmentSize != 0) {
    ++numberOfFragments;
  }

  if(numberOfFragments < std::numeric_limits<uint16_t>::max()) {
    for(unsigned int i = 0; i < numberOfFragments; ++i) {
      size_t startIndex = i * fragmentSize;
      
      //std::string's substr function will give us all characters until the end of
      //the string if we ask it for too many (shouldn't run off the end of the string)
      DataMessageFragment::MessageDataPtr fragmentData(new std::string(message.substr(startIndex, fragmentSize)));

      DataMessageFragment newFragment;
      newFragment.sequenceNumber = nextSequenceNumber;
      newFragment.index = i;
      newFragment.count = numberOfFragments;
      newFragment.shouldAck = shouldAck;
      newFragment.messageData = fragmentData;

      sendDataMap[nextSequenceNumber] = newFragment;

      ++nextSequenceNumber;
    }
  } else {
    LOG_ERROR("Tried to send a message with too many fragments");
  }
}

void SerialConnector::receivedAckPacket(const bool isToken, const std::vector<uint16_t> &acks) {
  if(isToken) {
    signalEvent(EVENT_TOKEN_RECEIVED);
  } else {
    signalEvent(EVENT_MESSAGE_RECEIVED);
    LOG_DEBUG("Received ack");

    {
      ThreadMutexGuard g(sendDataMapMutex);
      for(std::vector<uint16_t>::const_iterator it = acks.begin(); it != acks.end(); ++it) {
        size_t result = sendDataMap.erase(*it);
        if(result < 1) {
          LOG_WARN("Couldn't find acknowledged message.  Received duplicate ack?");
        }
      }
    }
  }
}

void SerialConnector::receivedReset() {
  signalEvent(EVENT_RESET_RECEIVED);
}

void SerialConnector::processMessage(const uint8_t dataType, const std::string &message) {

  //do protobuf decoding to get actual data type and data
  ammo::protocol::MessageWrapper protobufMessage;
  bool success = protobufMessage.ParseFromString(message);

  if(success) {
    if(protobufMessage.type() == ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE) {
      uint8_t actualType = protobufMessage.terse_message().mime_type();
      LOG_DEBUG("Received message of type " << (int) actualType);
      std::string actualData = protobufMessage.terse_message().data();
      terseDecoder.processMessage(actualType, actualData);
    } else {
      LOG_ERROR("Invalid protobuf message type " << protobufMessage.type());
    }
  } else {
    LOG_ERROR("Couldn't deserialize protobuf message");
  }
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

void SerialConnector::sendResetAck() {
  std::ostringstream resetPacketDataStream;
  appendUInt8(resetPacketDataStream, 0xc0); //1100 0000
  appendUInt16(resetPacketDataStream, 0); //reset packet always has a payload of a 16-bit integer, set to 0 for consistency with ack/token messages

  std::string resetPacketData = resetPacketDataStream.str();

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string(resetPacketDataStream.str()));
  writer.queueMessage(messageToSend);
}

void SerialConnector::sendAckPacket() {
  SequenceNumberQueue sequenceNumbers = getSequenceNumbersToAck();

  std::ostringstream ackPacketDataStream; //first byte is message type; second two bytes are ack info
  appendUInt8(ackPacketDataStream, 0x80); //1000 0000; ack or token packet
  appendUInt16(ackPacketDataStream, sequenceNumbers.size() & 0xefff); //mask off high byte; if it's set, this is a token (which it isn't)

  while(!sequenceNumbers.empty()) {
    appendUInt16(ackPacketDataStream, sequenceNumbers.front());
    sequenceNumbers.pop();
  }

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string(ackPacketDataStream.str()));

  writer.queueMessage(messageToSend);
}

void SerialConnector::sendTokenPacket() {
  std::ostringstream ackPacketDataStream; //first byte is message type; second two bytes are ack info
  appendUInt8(ackPacketDataStream, 0x80); //1000 0000; ack or token packet
  appendUInt16(ackPacketDataStream, 0x8000); //1000 0000 0000 0000; high byte set indicates that this is a token packet; no acks included

  SerialWriterThread::MutableQueuedMessagePtr messageToSend(new std::string(ackPacketDataStream.str()));

  LOG_TRACE("Sending token packet");
  writer.queueMessage(messageToSend);
}

void SerialConnector::sendDataFragments() {
  ThreadMutexGuard g(sendDataMapMutex);

  int sentFragments = 0;
  SendDataMap::iterator it = sendDataMap.begin();
  while(it != sendDataMap.end() && sentFragments < fragmentsToSendPerCycle) {
    LOG_DEBUG("Sending message fragment " << it->second.sequenceNumber << "(" << it->second.index << "/" << it->second.count << ")");
    writer.queueMessage(it->second.serializeFragment());
    SendDataMap::iterator previousItem = it++;
    if(!previousItem->second.shouldAck) {
      //we don't require an ack for this item, so delete it from the map
      sendDataMap.erase(previousItem); //invalidates previousItem, but not other iterators (like it)
    }
  }
}

void appendUInt8(std::ostream &stream, const uint8_t val) {
  stream.write(reinterpret_cast<const char *>(&val), sizeof(val));
}


void appendUInt16(std::ostream &stream, const uint16_t val) {
  stream.write(reinterpret_cast<const char *>(&val), sizeof(val));
}

//GatewayConnectorDelegate methods
void SerialConnector::onConnect(ammo::gateway::GatewayConnector *sender) {
  //FIXME: turned off for build for Brad (not quite working on device side)
  //sender->registerDataInterest("ammo/transapps.pli.locations", this);
  //sender->registerDataInterest("ammo/transapps.chat.message_groupAll", this);
}

void SerialConnector::onDisconnect(ammo::gateway::GatewayConnector *sender) {
  //do nothing
}

//DataPushReceiverListener methods
void SerialConnector::onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  TerseEncoder encoder;
  std::string terseOutput;

  bool result = encoder.encodeTerseFromJson(pushData.mimeType, pushData.data, terseOutput);
  if(result == true) {
    enqueueDataMessage(terseOutput, false); //todo: figure out which types should be acked
  } else {
    LOG_ERROR("Terse encoding failed");
  }
}
