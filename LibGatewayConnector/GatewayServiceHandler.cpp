#include "GatewayServiceHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "GatewayConnector.h"

#include "log.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

/* GatewayServiceHandler::GatewayServiceHandler(ACE_Thread_Manager *tm = 0, ACE_Message_Queue<ACE_NULL_SYNCH> *mq = 0, ACE_Reactor *reactor = ACE_Reactor::instance())
: ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>(tm, mq, reactor)
{
  
} */

int ammo::gateway::internal::GatewayServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_SIZE;
  dataSize = 0;
  checksum = 0;
  collectedData = NULL;
  position = 0;
  
  return 0;
}

int ammo::gateway::internal::GatewayServiceHandler::handle_input(ACE_HANDLE fd) {
  //LOG_TRACE("In handle_input");
  int count = 0;
  
  if(state == READING_SIZE) {
    count = this->peer().recv_n(&dataSize, sizeof(dataSize));
    //LOG_TRACE("SIZE Read " << count << " bytes");
  } else if(state == READING_CHECKSUM) {
    count = this->peer().recv_n(&checksum, sizeof(checksum));
    //LOG_TRACE("SUM Read " << count << " bytes");
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, dataSize - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR("Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_SIZE) {
      collectedData = new char[dataSize];
      position = 0;
      //LOG_TRACE("Got data size (" << dataSize << ")");
      state = READING_CHECKSUM;
    } else if(state == READING_CHECKSUM) {
      //LOG_TRACE("Got data checksum (" << checksum << ")");
      state = READING_DATA;
    } else if(state == READING_DATA) {
      //LOG_TRACE("Got some data...");
      position += count;
      if(position == dataSize) {
        //LOG_TRACE("Got all the data... processing");
        processData(collectedData, dataSize, checksum);
        //LOG_TRACE("Processsing complete.  Deleting buffer.");
        delete[] collectedData;
        collectedData = NULL;
        dataSize = 0;
        position = 0;
        state = READING_SIZE;
      }
    }
  } else if(count == 0) {
    LOG_ERROR("Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG_ERROR("Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  //LOG_TRACE("Leaving handle_input()");
  return 0;
}

// This method comes stright from ACE sample code:  see ace_wrappers/examples/APG/Reactor/Client.cpp
int ammo::gateway::internal::GatewayServiceHandler::handle_output(ACE_HANDLE) {
  ACE_Message_Block *mb;
  ACE_Time_Value nowait(ACE_OS::gettimeofday ());
  
  while (-1 != this->getq(mb, &nowait)) {
    ssize_t send_cnt = this->peer().send(mb->rd_ptr(), mb->length());
    if (send_cnt == -1) {
      LOG_ERROR("Send error...");
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

void ammo::gateway::internal::GatewayServiceHandler::sendData(ammo::gateway::protocol::GatewayWrapper &msg) {
  /*//this is bad--  if the message queue gets filled, putq will block forever
  //because there's not another thread pulling objects out of the queue.
  //TODO: figure out how to queue data infinitely without blocking
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
  
  if(msg.IsInitialized()) { //Don't send a message if it's missing required fields (SerializeToArray is supposed to check for this, but doesn't)
    msg.SerializeToArray(messageToSend, messageSize);
    unsigned int messageChecksum = ACE::crc32(messageToSend, messageSize);
    
    this->peer().send_n(&messageSize, sizeof(messageSize));
    this->peer().send_n(&messageChecksum, sizeof(messageChecksum));
    this->peer().send_n(messageToSend, messageSize);
  } else {
    LOG_ERROR("SEND ERROR (LibGatewayConnector):  Message is missing a required element.");
  }
  
  delete[] messageToSend;
}

int ammo::gateway::internal::GatewayServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR("Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)");
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::gateway::protocol::GatewayWrapper msg;
  bool result = msg.ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR("GatewayWrapper could not be deserialized.");
    LOG_ERROR("Client must have sent something that isn't a protocol buffer (or the wrong type).");
    return -1;
  }
  //LOG_INFO("Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    LOG_DEBUG("Received Associate Result...");
    parent->onAssociateResultReceived(msg.associate_result());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    parent->onPushDataReceived(msg.push_data());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request...");
    parent->onPullRequestReceived(msg.pull_request());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE) {
    LOG_DEBUG("Received Pull Response...");
    parent->onPullResponseReceived(msg.pull_response());
  }
  
  return 0;
}

ammo::gateway::internal::GatewayServiceHandler::~GatewayServiceHandler() {
  LOG_TRACE("GatewayServiceHandler being destroyed!");
}

void ammo::gateway::internal::GatewayServiceHandler::setParentConnector(GatewayConnector *parent) {
  this->parent = parent;
}
