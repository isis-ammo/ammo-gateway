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
  
  this->peer().enable(ACE_NONBLOCK);
  
  return 0;
}

int AndroidServiceHandler::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask m) {
  connectionClosing = true;
  LOG_TRACE(this << " Closing Message Processor");
  messageProcessor->close(0);
  LOG_TRACE(this << " Waiting for message processor thread to finish...");
  this->reactor()->lock().release(); //release the lock, or we'll hang if other threads try to do stuff with the reactor while we're waiting
  messageProcessor->wait();
  this->reactor()->lock().acquire(); //and put it back like it was
  LOG_TRACE(this << " Message processor finished.");
  super::handle_close(fd, m);
  
  return 0;
}

int AndroidServiceHandler::handle_input(ACE_HANDLE fd) {
  LOG_TRACE(this << " In handle_input");
  int count = 0;
  
  if(state == READING_HEADER) {
    count = this->peer().recv_n(&messageHeader, sizeof(messageHeader));
    //verify the message header (check its magic number and checksum)
    if(messageHeader.magicNumber == HEADER_MAGIC_NUMBER) {
      unsigned int calculatedChecksum = ACE::crc32(&messageHeader, sizeof(messageHeader) - sizeof(messageHeader.headerChecksum));
      if(calculatedChecksum != messageHeader.headerChecksum) {
        LOG_ERROR("Invalid header checksum");
      }
    } else {
      LOG_ERROR("Invalid magic number: 0x" << hex << messageHeader.magicNumber << dec);
    }
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, messageHeader.size - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR(this << " Invalid state!");
  }
  
  
  
  if(count > 0) {
    if(state == READING_HEADER) {
      collectedData = new char[messageHeader.size];
      position = 0;
      //LOG_TRACE("Got data size (" << dataSize << ")");
      state = READING_DATA;
    } else if(state == READING_DATA) {
      LOG_TRACE(this << " Got some data...");
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
    LOG_INFO(this << " Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG_ERROR(this << " Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  LOG_TRACE(this << " Leaving handle_input()");
  return 0;
}

int AndroidServiceHandler::handle_output(ACE_HANDLE fd) {
  int count = 0;
  
  do {
    if(dataToSend == NULL) {
      ammo::protocol::MessageWrapper *msg = getNextMessageToSend();
      if(msg != NULL) {
        LOG_TRACE("Getting a new message to send");
        if(!msg->IsInitialized()) {
          LOG_WARN(this << " Protocol Buffers message is missing a required element.");
        }
        unsigned int messageSize = msg->ByteSize();
        sendBufferSize = messageSize + sizeof(MessageHeader);
        dataToSend = new char[sendBufferSize];
        MessageHeader *headerToSend = (MessageHeader *) dataToSend;
        headerToSend->magicNumber = HEADER_MAGIC_NUMBER;
        headerToSend->size = messageSize;
        
        char *protobufSerializedMessage = dataToSend + sizeof(MessageHeader);
        msg->SerializeToArray(protobufSerializedMessage, messageSize);
        
        headerToSend->checksum = ACE::crc32(protobufSerializedMessage, messageSize);
        headerToSend->headerChecksum = ACE::crc32(headerToSend, 3*sizeof(int));
        
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
    LOG_ERROR(this << " Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  
  return 0;
}

int AndroidServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR(this << " Mismatched checksum " << std::hex << calculatedChecksum << " : " << messageChecksum);
    LOG_ERROR(this << " size " << std::dec << messageSize ); // << " payload: " < );
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR(this << " MessageWrapper could not be deserialized.");
    LOG_ERROR(this << " Client must have sent something that isn't a protocol buffer (or the wrong type).");
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
    LOG_TRACE(this << " Queued a message to send.  " << sendQueue.size() << " messages in queue.");
  }
  sendQueueMutex.release();
  if(!connectionClosing) {
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  }
}

ammo::protocol::MessageWrapper *AndroidServiceHandler::getNextMessageToSend() {
  ammo::protocol::MessageWrapper *msg = NULL;
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    msg = sendQueue.front();
    sendQueue.pop();
  }
  int size = sendQueue.size();
  sendQueueMutex.release();
  //if(size > 0) {
    LOG_TRACE(this << " Dequeued a message to send.  " << size << " messages remain in queue.");
  //}
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
  LOG_TRACE(this << " In ~AndroidServiceHandler");
  delete messageProcessor;
}
