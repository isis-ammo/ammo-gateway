#include "SerialConnector.h"

#include <sstream>

#include "SatcomConfigurationManager.h"

void FragmentedMessage::gotMessageFragment(const DataMessage dataHeader, const std::string &data) {
  //Validate message count
  if(dataHeader.count != fragmentsCount) {
    LOG_ERROR("Fragment count mismatch; dropping (header = " << dataHeader.count << " stored = " << fragmentsCount << ")");
  } else {
    if(fragments[dataHeader.index]) {
      LOG_ERROR("Received duplicate fragment " << dataHeader.index << " of " << fragmentsCount);
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
closed(true),
serialDevice(),
serialConnector()
{
  
}

SerialConnector::~SerialConnector() {
  if(!isClosed()) {
    LOG_ERROR("SerialReaderThread: Entered destructor before stop()");
    stop();
    this->wait();
  }
}

int SerialConnector::svc() {
  bool status = connect();

  if(status == false) {
   return 1;
  }

  while(!isClosed()) {

  }

  return 0;
}

bool SerialConnector::connect() {
  const std::string listenPort = SatcomConfigurationManager::getInstance().getListenPort();

  //ACE-based serial initialization code
  int result = serialConnector.connect(serialDevice, ACE_DEV_Addr(listenPort.c_str()));
  if(result == -1) {
    LOG_ERROR("Couldn't open serial port " << listenPort << " (" << errno << ": " << strerror(errno) << ")" );
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
    LOG_ERROR("Couldn't configure serial port");
    return false;
  }

  return true;
}

void SerialConnector::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
  } else {
    LOG_ERROR("Error acquiring lock in SerialConnector::stop()")
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
      LOG_ERROR("Error acquiring lock in SerialConnector::isClosed()")
    }
  }
  return temp;
}

char SerialConnector::readChar() {
  unsigned char temp;
  
  ssize_t count = 0;
  while(count == 0 || (count == -1 && errno == ETIME)) {
    ACE_Time_Value timeout(0, 10000);
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
  return temp;
}

bool SerialConnector::writeMessageFragment(const std::string &message) {
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
    processMessage(completeMessage);
    incompleteMessages.erase(messageIt);
  }

  if(shouldAck == 1) {
    ThreadMutexGuard g(sequenceNumbersToAckMutex);
    sequenceNumbersToAck.push(dataHeader.sequenceNumber);
  }
}

void SerialConnector::receivedAckPacket(const bool isToken, const std::vector<uint16_t> &acks) {
  //TODO: implement me
}

void SerialConnector::processMessage(std::string &message) {
  //TODO:  do terse decoding and forward on to gateway
}

// SequenceNumberQueue SerialConnector::getSequenceNumbersToAck() {

// }



