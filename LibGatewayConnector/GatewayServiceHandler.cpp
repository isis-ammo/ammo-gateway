#include "GatewayServiceHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "GatewayConnector.h"

#include "log.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

using namespace std;

/* GatewayServiceHandler::GatewayServiceHandler(ACE_Thread_Manager *tm = 0, ACE_Message_Queue<ACE_NULL_SYNCH> *mq = 0, ACE_Reactor *reactor = ACE_Reactor::instance())
: ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>(tm, mq, reactor)
{
  
} */

int ammo::gateway::internal::GatewayServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_HEADER;
  collectedData = NULL;
  position = 0;
  
  dataToSend = NULL;
  sendPosition = 0;
  sendBufferSize = 0;
  
  return 0;
}

int ammo::gateway::internal::GatewayServiceHandler::handle_input(ACE_HANDLE fd) {
  //LOG_TRACE("In handle_input");
  int count = 0;
  
  if(state == READING_HEADER) {
    count = this->peer().recv_n(&header, sizeof(header));
    //LOG_TRACE("SIZE Read " << count << " bytes");
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, header.size - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR("Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_HEADER) {
      //validate magic number
      //validate header checksum
      if(header.magicNumber == HEADER_MAGIC_NUMBER) {
        unsigned int calculatedChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));
        if(calculatedChecksum != header.headerChecksum) {
          LOG_ERROR(this << " Invalid header checksum");
          return -1;
        }
        
        try {
          collectedData = new char[header.size];
        } catch (std::bad_alloc &e) {
          LOG_ERROR(this << " Couldn't allocate memory for message of size " << header.size);
          return -1;
        }
        position = 0;
        //LOG_TRACE("Got data size (" << dataSize << ")");
        state = READING_DATA;
      } else {
        LOG_ERROR((long) this << " Invalid magic number: " << hex << header.magicNumber << dec);
        return -1;
      }
    } else if(state == READING_DATA) {
      //LOG_TRACE("Got some data...");
      position += count;
      if(position == header.size) {
        LOG_TRACE("Got a whole message...  processing");
        //LOG_TRACE("Got all the data... processing");
        processData(collectedData, header.size, header.checksum);
        //LOG_TRACE("Processsing complete.  Deleting buffer.");
        delete[] collectedData;
        collectedData = NULL;
        header.size = 0;
        header.headerChecksum = 0;
        header.magicNumber = 0;
        position = 0;
        state = READING_HEADER;
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

int ammo::gateway::internal::GatewayServiceHandler::handle_output(ACE_HANDLE fd) {
  int count = 0;
  
  do {
    if(dataToSend == NULL) {
      ammo::gateway::protocol::GatewayWrapper *msg = getNextMessageToSend();
      if(msg != NULL) {
        LOG_TRACE("Getting a new message to send");
        if(!msg->IsInitialized()) {
          LOG_WARN("Protocol Buffers message is missing a required element.");
        }
        unsigned int messageSize = msg->ByteSize();
        sendBufferSize = messageSize + sizeof(MessageHeader);
        dataToSend = new char[sendBufferSize];
        MessageHeader *headerToSend = (MessageHeader *) dataToSend;
        headerToSend->magicNumber = HEADER_MAGIC_NUMBER;
        headerToSend->size = messageSize;
        headerToSend->reserved[0] = 0;
        headerToSend->reserved[1] = 0;
        headerToSend->reserved[2] = 0;
        headerToSend->reserved[3] = 0;
        
        char *protobufSerializedMessage = dataToSend + sizeof(MessageHeader);
        
        headerToSend->size = messageSize;
        msg->SerializeToArray(protobufSerializedMessage, messageSize);
        headerToSend->checksum = ACE::crc32(protobufSerializedMessage, messageSize);
        headerToSend->headerChecksum = ACE::crc32(headerToSend, sizeof(MessageHeader) - sizeof(headerToSend->headerChecksum));
        
        sendPosition = 0;
        
        delete msg;
      } else {
        //don't wake up the reactor when there's no data that needs to be sent
        //(wake-up will be rescheduled when data becomes available in sendMessage
        //below)
        this->reactor()->cancel_wakeup(this, ACE_Event_Handler::WRITE_MASK);
        return 0;
      }
    }
      
    //timeout after ten seconds when sending data (in case connection drops
    //in the middle, we don't want to wait until the socket connection dies)
    //ACE_Time_Value timeout(10);
    count = this->peer().send(dataToSend + sendPosition, sendBufferSize - sendPosition);
    if(count >= 0) {
      sendPosition += count;
    }
    LOG_TRACE("Sent " << count << " bytes (current postition " << sendPosition << "/" << sendBufferSize);
    
    if(sendPosition >= (sendBufferSize)) {
      delete[] dataToSend;
      dataToSend = NULL;
      sendBufferSize = 0;
      sendPosition = 0;
    }
  } while(count != -1);
  
  if(count == -1 && ACE_OS::last_error () == EWOULDBLOCK) {
    LOG_TRACE("Received EWOULDBLOCK");
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  } else {
    LOG_ERROR("Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  
  return 0;
}

void ammo::gateway::internal::GatewayServiceHandler::sendData(ammo::gateway::protocol::GatewayWrapper *msg) {
  sendQueueMutex.acquire();
  sendQueue.push(msg);
  LOG_TRACE("Queued a message to send.  " << sendQueue.size() << " messages in queue.");
  sendQueueMutex.release();
  
  this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
}

ammo::gateway::protocol::GatewayWrapper *ammo::gateway::internal::GatewayServiceHandler::getNextMessageToSend() {
  ammo::gateway::protocol::GatewayWrapper *msg = NULL;
  
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    msg = sendQueue.front();
    sendQueue.pop();
  }
  
  int size = sendQueue.size();
  sendQueueMutex.release();
  
  LOG_TRACE("Dequeued a message to send.  " << size << " messages remain in queue.");
  return msg;
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
