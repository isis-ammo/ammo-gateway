#include "AndroidServiceHandler.h"
#include "protocol/AmmmoMessages.pb.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

extern std::string gatewayAddress;
extern int gatewayPort;

int AndroidServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  
  gatewayConnector = new GatewayConnector(this);
}

int AndroidServiceHandler::handle_input(ACE_HANDLE fd) {
  //std::cout << "In handle_input" << std::endl << std::flush;
  int count = 0;
  
  if(state == READING_SIZE) {
    count = this->peer().recv_n(&dataSize, sizeof(dataSize));
    //std::cout << "SIZE Read " << count << " bytes" << std::endl << std::flush;
  } else if(state == READING_CHECKSUM) {
    count = this->peer().recv_n(&checksum, sizeof(checksum));
    //std::cout << "SUM Read " << count << " bytes" << std::endl << std::flush;
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, dataSize - position);
    //std::cout << "DATA Read " << count << " bytes" << std::endl << std::flush;
  } else {
    std::cout << "Invalid state!" << std::endl << std::flush;
  }
  
  
  
  if(count > 0) {
    if(state == READING_SIZE) {
      collectedData = new char[dataSize];
      position = 0;
      //std::cout << "Got data size (" << dataSize << ")" << std::endl << std::flush;
      state = READING_CHECKSUM;
    } else if(state == READING_CHECKSUM) {
      //std::cout << "Got data checksum (" << checksum << ")" << std::endl << std::flush;
      state = READING_DATA;
    } else if(state == READING_DATA) {
      //std::cout << "Got some data..." << std::endl << std::flush;
      position += count;
      if(position == dataSize) {
        //std::cout << "Got all the data... processing" << std::endl << std::flush;
        processData(collectedData, dataSize, checksum);
        //std::cout << "Processsing complete.  Deleting buffer." << std::endl << std::flush;
        delete[] collectedData;
        collectedData = NULL;
        dataSize = 0;
        position = 0;
        state = READING_SIZE;
      }
    }
  } else if(count == 0) {
    std::cout << "Connection closed." << std::endl << std::flush;
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    std::cout << "Socket error occurred. (" << ACE_OS::last_error() << ")" << std::endl << std::flush;
    return -1;
  }
  //std::cout << "Leaving handle_input()" << std::endl << std::flush;
  return 0;
}

void AndroidServiceHandler::sendData(ammmo::protocol::MessageWrapper &msg) {
  /*Fixme: potential deadlock here
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  msg.SerializeToArray(messageToSend, messageSize);
  unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
  
  ACE_Message_Block *messageSizeBlock = new ACE_Message_Block(sizeof(messageSize));
  messageSizeBlock->copy((char *) &messageSize, sizeof(messageSize));
  this->putq(messageSizeBlock);
  
  ACE_Message_Block *messageChecksumBlock = new ACE_Message_Block(sizeof(messageChecksum));
  messageChecksumBlock->copy((char *) &messageChecksum, sizeof(messageChecksum));
  this->putq(messageChecksumBlock);
  
  ACE_Message_Block *messageToSendBlock = new ACE_Message_Block(messageSize);
  messageToSendBlock->copy(messageToSend, messageSize);
  this->putq(messageToSendBlock);
  
  this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);*/
  
  unsigned int messageSize = msg.ByteSize();
  char *messageToSend = new char[messageSize];
  msg.SerializeToArray(messageToSend, messageSize);
  unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
  
  this->peer().send_n(&messageSize, sizeof(messageSize));
  this->peer().send_n(&messageChecksum, sizeof(messageChecksum));
  this->peer().send_n(messageToSend, messageSize);
}

int AndroidServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    std::cout << "Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)" << std::endl << std::flush;
    return -1;
  }
  
  //checksum is valid; parse the data
  ammmo::protocol::MessageWrapper msg;
  bool result = msg.ParseFromArray(data, messageSize);
  if(result == false) {
    std::cout << "MessageWrapper could not be deserialized." << std::endl;
    std::cout << "Client must have sent something that isn't a protocol buffer (or the wrong type)." << std::endl << std::flush;
    return -1;
  }
  //std::cout << "Message Received: " << msg.DebugString() << std::endl << std::flush;
  
  if(msg.type() == ammmo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    std::cout << "Received Authentication Message..." << std::endl << std::flush;
    if(gatewayConnector != NULL) {
      ammmo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
    }
  } else if(msg.type() == ammmo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    std::cout << "Received Data Message..." << std::endl << std::flush;
    if(gatewayConnector != NULL) {
      ammmo::protocol::DataMessage dataMessage = msg.data_message();
      gatewayConnector->pushData(dataMessage.uri(), dataMessage.mime_type(), dataMessage.data());
    }
  } else if(msg.type() == ammmo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    std::cout << "Received Subscribe Message..." << std::endl << std::flush;
    if(gatewayConnector != NULL) {
      ammmo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this);
    }
  }
  
  return 0;
}

void AndroidServiceHandler::onConnect(GatewayConnector *sender) {
  
}

void AndroidServiceHandler::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidServiceHandler::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data) {
  std::cout << "Sending subscribed data to device..." << std::endl;
  std::cout << "   URI: " << uri << ", Type: " << mimeType << std::endl << std::flush;;
  
  std::string dataString(data.begin(), data.end());
  ammmo::protocol::MessageWrapper msg;
  ammmo::protocol::DataMessage *dataMsg = msg.mutable_data_message();
  dataMsg->set_uri(uri);
  dataMsg->set_mime_type(mimeType);
  dataMsg->set_data(dataString);
  
  msg.set_type(ammmo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  
  std::cout << "Sending Data Push message to connected plugin" << std::endl << std::flush;
  this->sendData(msg);
}

void AndroidServiceHandler::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  std::cout << "Delegate: onAuthenticationResponse" << std::endl << std::flush;
  ammmo::protocol::MessageWrapper newMsg;
  newMsg.set_type(ammmo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg.mutable_authentication_result()->set_result(result ? ammmo::protocol::AuthenticationResult_Status_SUCCESS : ammmo::protocol::AuthenticationResult_Status_SUCCESS);
  this->sendData(newMsg);
}

AndroidServiceHandler::~AndroidServiceHandler() {
  delete gatewayConnector;
}
