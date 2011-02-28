#include "AndroidServiceHandler.h"
#include "protocol/AmmoMessages.pb.h"
#include "AndroidMessageProcessor.h"

#include <iostream>

#include "ace/OS_NS_errno.h"

#include "log.h"

using namespace std;

extern std::string gatewayAddress;
extern int gatewayPort;

AndroidServiceHandler::AndroidServiceHandler() : 
messageProcessor(NULL),
sendQueueMutex(), 
receiveQueueMutex()
{

}

int AndroidServiceHandler::open(void *ptr) {
  if(super::open(ptr) == -1) {
    return -1;
    
  }
  state = READING_HEADER;
  collectedData = NULL;
  position = 0;
  
  dataToSend = NULL;
  position = 0;
  
  messageHeader.magicNumber = 0;
  messageHeader.size = 0;
  messageHeader.checksum = 0;
  messageHeader.headerChecksum = 0;
  
  connectionClosing = false;
  
  messageProcessor = new AndroidMessageProcessor(this);
  messageProcessor->activate();
  
  return 0;
}

int AndroidServiceHandler::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask m) {
  connectionClosing = true;
  LOG_TRACE("Closing Message Processor");
  messageProcessor->close(0);
  LOG_TRACE("Waiting for message processor thread to finish...");
  messageProcessor->wait();
  LOG_TRACE("Message processor finished.");
  super::handle_close(fd, m);
  
  return 0;
}

int AndroidServiceHandler::handle_input(ACE_HANDLE fd) {
  LOG_TRACE("In handle_input");
  int count = 0;
  
  if(state == READING_HEADER) {
    count = this->peer().recv_n(&messageHeader, sizeof(messageHeader));
    //verify the message header (check its magic number and checksum)
    if(messageHeader.magicNumber == 0xfeedbeef) {
      unsigned int calculatedChecksum = ACE::crc32(&messageHeader, 3*sizeof(int));
      if(calculatedChecksum != messageHeader.headerChecksum) {
        LOG_ERROR("Invalid header checksum!");
      }
    } else {
      LOG_ERROR("Invalid magic number!");
    }
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, messageHeader.size - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR("Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_HEADER) {
      collectedData = new char[messageHeader.size];
      position = 0;
      //LOG_TRACE("Got data size (" << dataSize << ")");
      state = READING_DATA;
    } else if(state == READING_DATA) {
      //LOG_TRACE("Got some data...");
      position += count;
      if(position == messageHeader.size) {
        //LOG_TRACE("Got all the data... processing");
        processData(collectedData, messageHeader.size, messageHeader.checksum);
        //LOG_TRACE("Processsing complete.  Deleting buffer.");
        delete[] collectedData;
        collectedData = NULL;
        messageHeader.magicNumber = 0;
        messageHeader.size = 0;
        messageHeader.checksum = 0;
        messageHeader.headerChecksum = 0;
        position = 0;
        state = READING_HEADER;
      }
    }
  } else if(count == 0) {
    LOG_INFO("Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG_ERROR("Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  //LOG_TRACE("Leaving handle_input()");
  return 0;
}

int AndroidServiceHandler::handle_output(ACE_HANDLE fd) {
  int count = 0;
  
  do {
    if(dataToSend == NULL) {
      ammo::protocol::MessageWrapper *msg = getNextMessageToSend();
      if(msg != NULL) {
        if(!msg->IsInitialized()) {
          LOG_WARN("Protocol Buffers message is missing a required element.");
        }
        unsigned int messageSize = msg->ByteSize();
        sendBufferSize = messageSize + 2*sizeof(unsigned int);
        dataToSend = new char[sendBufferSize];
        unsigned int *size = (unsigned int *) dataToSend;
        unsigned int *messageChecksum = (unsigned int *) (dataToSend + sizeof(unsigned int));
        char *protobufSerializedMessage = dataToSend + 2*sizeof(unsigned int);
        
        *size = messageSize;
        msg->SerializeToArray(protobufSerializedMessage, messageSize);
        *messageChecksum = ACE::crc32(protobufSerializedMessage, messageSize);
        
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
      
    count = this->peer().send(dataToSend, sendBufferSize - sendPosition);
    if(count >= 0) {
      sendPosition += count;
    }
    
    if(sendPosition >= (sendBufferSize - 1)) {
      delete[] dataToSend;
      dataToSend = NULL;
      sendBufferSize = 0;
      sendPosition = 0;
    }
  } while(count != -1);
  
  if(count == -1 && ACE_OS::last_error () == EWOULDBLOCK) {
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  } else {
    LOG_ERROR("Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  
  return 0;
}

int AndroidServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR("Invalid checksum--  we've been sent bad data (perhaps a message size mismatch?)");
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR("MessageWrapper could not be deserialized.");
    LOG_ERROR("Client must have sent something that isn't a protocol buffer (or the wrong type).");
    delete msg;
    return -1;
  }
  addReceivedMessage(msg);
  messageProcessor->signalNewMessageAvailable();
  
  return 0;
}

void AndroidServiceHandler::sendMessage(ammo::protocol::MessageWrapper *msg) {
  sendQueueMutex.acquire();
  if(!connectionClosing) {
    sendQueue.push(msg);
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
    LOG_TRACE("Queued a message to send.  " << sendQueue.size() << " messages in queue.");
  }
  sendQueueMutex.release();
}

ammo::protocol::MessageWrapper *AndroidServiceHandler::getNextMessageToSend() {
  ammo::protocol::MessageWrapper *msg = NULL;
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    msg = sendQueue.front();
    sendQueue.pop();
  }
  sendQueueMutex.release();
  LOG_TRACE("Dequeued a message to send.  " << sendQueue.size() << " messages remain in queue.");
  return msg;
}

ammo::protocol::MessageWrapper *AndroidServiceHandler::getNextReceivedMessage() {
  ammo::protocol::MessageWrapper *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.front();
    receiveQueue.pop();
  }
  receiveQueueMutex.release();
  
  return msg;
}

void AndroidServiceHandler::addReceivedMessage(ammo::protocol::MessageWrapper *msg) {
  receiveQueueMutex.acquire();
  receiveQueue.push(msg);
  receiveQueueMutex.release();
}

AndroidServiceHandler::~AndroidServiceHandler() {
  LOG_TRACE("In ~AndroidServiceHandler");
  delete messageProcessor;
}
