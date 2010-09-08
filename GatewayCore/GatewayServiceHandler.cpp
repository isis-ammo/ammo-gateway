#include "GatewayServiceHandler.h"
#include "GatewayCore.h"
#include "protocol/GatewayPrivateMessages.pb.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

/* GatewayServiceHandler::GatewayServiceHandler(ACE_Thread_Manager *tm = 0, ACE_Message_Queue<ACE_NULL_SYNCH> *mq = 0, ACE_Reactor *reactor = ACE_Reactor::instance())
: ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>(tm, mq, reactor)
{
  
} */

int GatewayServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  
  //return 0;
}

int GatewayServiceHandler::handle_input(ACE_HANDLE fd) {
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
        delete collectedData;
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

// This method comes stright from ACE sample code:  see ace_wrappers/examples/APG/Reactor/Client.cpp
int GatewayServiceHandler::handle_output(ACE_HANDLE) {
  ACE_Message_Block *mb;
  ACE_Time_Value nowait(ACE_OS::gettimeofday ());
  
  while (-1 != this->getq(mb, &nowait)) {
    ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
    if (send_cnt == -1) {
      std::cout << "Send error..." << std::endl << std::flush;
    } else {
      mb->rd_ptr(static_cast<size_t>(send_cnt));
    }
    if (mb->length() > 0) {
      this->ungetq(mb);
      break;
    }
    mb->release();
  }
  
  if (this->msg_queue ()->is_empty ()) {
    this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  } else {
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  }
  
  return 0;
}

void GatewayServiceHandler::sendData(ammmo::gateway::protocol::GatewayWrapper &msg) {
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

int GatewayServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    std::cout << "Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)" << std::endl << std::flush;
    return -1;
  }
  
  //checksum is valid; parse the data
  ammmo::gateway::protocol::GatewayWrapper msg;
  bool result = msg.ParseFromArray(data, messageSize);
  if(result == false) {
    std::cout << "GatewayWrapper could not be deserialized." << std::endl;
    std::cout << "Client must have sent something that isn't a protocol buffer (or the wrong type)." << std::endl << std::flush;
    return -1;
  }
  std::cout << "Message Received: " << msg.DebugString() << std::endl << std::flush;
  
  if(msg.type() == ammmo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE) {
    //TODO: split out into a different function and do more here
    ammmo::gateway::protocol::GatewayWrapper newMsg;
    newMsg.set_type(ammmo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT);
    newMsg.mutable_associate_result()->set_result(ammmo::gateway::protocol::AssociateResult_Status_SUCCESS);
    this->sendData(newMsg);
  } else if(msg.type() == ammmo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST) {
    std::string mime_type = msg.register_data_interest().mime_type();
    bool result = GatewayCore::getInstance()->registerDataInterest(mime_type, this);
    if(result == true) {
      registeredHandlers.push_back(mime_type);
    }
  } else if(msg.type() == ammmo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST) {
    std::string mime_type = msg.unregister_data_interest().mime_type();
    bool result = GatewayCore::getInstance()->unregisterDataInterest(mime_type, this);
    if(result == true) {
      for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
        if((*it) == mime_type) {
          registeredHandlers.erase(it);
        }
      }
    }
  } else if(msg.type() == ammmo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    bool result = GatewayCore::getInstance()->pushData(msg.push_data().uri(), msg.push_data().mime_type(), msg.push_data().data());
  }
  
  return 0;
}

bool GatewayServiceHandler::sendPushedData(std::string uri, std::string mimeType, const std::string &data) {
  ammmo::gateway::protocol::GatewayWrapper msg;
  ammmo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  
  msg.set_type(ammmo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  std::cout << "Sending Data Push message to connected plugin" << std::endl << std::flush;
  this->sendData(msg);
  return true;
}

GatewayServiceHandler::~GatewayServiceHandler() {
  std::cout << "GatewayServiceHandler being destroyed!" << std::endl << std::flush;
  std::cout << "Unregistering data handlers..." << std::endl << std::flush;
  for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
    GatewayCore::getInstance()->unregisterDataInterest(*it, this);
  }
}
