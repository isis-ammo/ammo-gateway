#include "AndroidServiceHandler.h"
#include "protocol/AmmmoMessages.pb.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

int AndroidServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  
  gatewayConnector = new GatewayConnector(NULL);
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
  std::cout << "Message Received: " << msg.DebugString() << std::endl << std::flush;
  
  if(msg.type() == ammmo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    if(gatewayConnector != NULL) {
      ammmo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
    }
  }
  
  return 0;
}

AndroidServiceHandler::~AndroidServiceHandler() {
  delete gatewayConnector;
}
