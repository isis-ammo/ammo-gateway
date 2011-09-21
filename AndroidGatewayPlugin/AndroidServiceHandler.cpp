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
  
  sentMessageCount = 0;
  receivedMessageCount = 0;
  
  connectionClosing = false;
  
  ACE_INET_Addr remoteAddress;
  int result = this->peer().get_remote_addr(remoteAddress);
  if(result == 0) {
    string printableRemoteAddress(remoteAddress.get_host_addr());
    LOG_INFO((long) this << " Got connection from " << printableRemoteAddress);
  } else {
    LOG_WARN((long) this << " Got new connection, but couldn't determine remote address.");
  }
  
  messageProcessor = new AndroidMessageProcessor(this);
  messageProcessor->activate();
  
  this->peer().enable(ACE_NONBLOCK);
  
  return 0;
}

int AndroidServiceHandler::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask m) {
  connectionClosing = true;
  LOG_TRACE((long) this << " Closing Message Processor");
  messageProcessor->close(0);
  LOG_TRACE((long) this << " Waiting for message processor thread to finish...");
  this->reactor()->lock().release(); //release the lock, or we'll hang if other threads try to do stuff with the reactor while we're waiting
  messageProcessor->wait();
  this->reactor()->lock().acquire(); //and put it back like it was
  LOG_TRACE((long) this << " Message processor finished.");
  super::handle_close(fd, m);
  
  return 0;
}

int AndroidServiceHandler::handle_input(ACE_HANDLE fd) {
  LOG_TRACE((long) this << " In handle_input");
  int count = 0;
  
  if(state == READING_HEADER) {
    count = this->peer().recv(&messageHeader, sizeof(messageHeader) - position);
  } else if(state == READING_DATA) {
    count = this->peer().recv(collectedData + position, messageHeader.size - position);
    //LOG_TRACE("DATA Read " << count << " bytes");
  } else {
    LOG_ERROR((long) this << " Invalid state!");
    return -1;
  }
  
  
  
  if(count > 0) {
    if(state == READING_HEADER) {
      position += count;
      if(position == sizeof(messageHeader)) {
        if(messageHeader.magicNumber == HEADER_MAGIC_NUMBER) {
          unsigned int calculatedChecksum = ACE::crc32(&messageHeader, sizeof(messageHeader) - sizeof(messageHeader.headerChecksum));
          if(calculatedChecksum != messageHeader.headerChecksum) {
            LOG_ERROR((long) this << " Invalid header checksum");
            sendErrorPacket(INVALID_HEADER_CHECKSUM);
            return -1;
          }
        } else {
          LOG_ERROR((long) this << " Invalid magic number: " << hex << messageHeader.magicNumber << dec);
          sendErrorPacket(INVALID_MAGIC_NUMBER);
          return -1;
        }
        
        try {
          collectedData = new char[messageHeader.size];
        } catch (std::bad_alloc &e) {
          LOG_ERROR((long) this << " Couldn't allocate memory for message of size " << messageHeader.size);
          sendErrorPacket(MESSAGE_TOO_LARGE);
          return -1;
        }
        position = 0;
        //LOG_TRACE("Got data size (" << dataSize << ")");
        state = READING_DATA;
      }
    } else if(state == READING_DATA) {
      LOG_TRACE((long) this << " Got some data...");
      position += count;
      if(position == messageHeader.size) {
        //LOG_TRACE("Got all the data... processing");
        processData(collectedData, messageHeader.size, messageHeader.checksum, messageHeader.priority);
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
    LOG_INFO((long) this << " Connection closed.");
    return -1;
  } else if(count == -1 && ACE_OS::last_error () != EWOULDBLOCK) {
    LOG_ERROR((long) this << " Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  LOG_TRACE((long) this << " Leaving handle_input()");
  return 0;
}

int AndroidServiceHandler::handle_output(ACE_HANDLE fd) {
  int count = 0;
  
  do {
    if(dataToSend == NULL) {
      ammo::protocol::MessageWrapper *msg = getNextMessageToSend();
      if(msg != NULL) {
        LOG_TRACE((long) this << " Getting a new message to send");
        if(!msg->IsInitialized()) {
          LOG_WARN((long) this << " Protocol Buffers message is missing a required element.");
        }
        unsigned int messageSize = msg->ByteSize();
        sendBufferSize = messageSize + sizeof(MessageHeader);
        dataToSend = new char[sendBufferSize];
        MessageHeader *headerToSend = (MessageHeader *) dataToSend;
        headerToSend->magicNumber = HEADER_MAGIC_NUMBER;
        headerToSend->size = messageSize;
        headerToSend->priority = msg->message_priority();
        headerToSend->error = NO_ERROR;
        headerToSend->reserved[0] = 0;
        headerToSend->reserved[1] = 0;
        
        char *protobufSerializedMessage = dataToSend + sizeof(MessageHeader);
        msg->SerializeToArray(protobufSerializedMessage, messageSize);
        
        headerToSend->checksum = ACE::crc32(protobufSerializedMessage, messageSize);
        headerToSend->headerChecksum = ACE::crc32(headerToSend, sizeof(messageHeader) - sizeof(messageHeader.headerChecksum));
        
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
    LOG_TRACE((long) this << " Sent " << count << " bytes (current postition " << sendPosition << "/" << sendBufferSize);
    
    if(sendPosition >= (sendBufferSize)) {
      delete[] dataToSend;
      dataToSend = NULL;
      sendBufferSize = 0;
      sendPosition = 0;
    }
  } while(count != -1);
  
  if(count == -1 && ACE_OS::last_error () == EWOULDBLOCK) {
    LOG_TRACE((long) this << " Received EWOULDBLOCK");
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  } else {
    LOG_ERROR((long) this << " Socket error occurred. (" << ACE_OS::last_error() << ")");
    return -1;
  }
  
  return 0;
}

int AndroidServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum, char priority) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR((long) this << " Mismatched checksum " << std::hex << calculatedChecksum << " : " << messageChecksum);
    LOG_ERROR((long) this << " size " << std::dec << messageSize ); // << " payload: " < );
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR((long) this << " MessageWrapper could not be deserialized.");
    LOG_ERROR((long) this << " Client must have sent something that isn't a protocol buffer (or the wrong type).");
    delete msg;
    return -1;
  }
  addReceivedMessage(msg, priority);
  messageProcessor->signalNewMessageAvailable();
  
  return 0;
}

void AndroidServiceHandler::sendMessage(ammo::protocol::MessageWrapper *msg, char priority) {
  QueuedMessage queuedMsg;
  queuedMsg.priority = priority;
  queuedMsg.message = msg;
  
  if(priority != msg->message_priority()) {
    LOG_WARN((long) this << " Priority mismatch when adding message to send queue: Header = " << (int) priority << ", Message = " << msg->message_priority());
  }
  
  sendQueueMutex.acquire();
  queuedMsg.messageCount = sentMessageCount;
  sentMessageCount++;
  if(!connectionClosing) {
    sendQueue.push(queuedMsg);
    LOG_TRACE((long) this << " Queued a message to send.  " << sendQueue.size() << " messages in queue.");
  }
  sendQueueMutex.release();
  if(!connectionClosing) {
    this->reactor()->schedule_wakeup(this, ACE_Event_Handler::WRITE_MASK);
  }
}

void AndroidServiceHandler::sendErrorPacket(char errorCode) {
  LOG_WARN((long) this << " Sending error packet to connected device");
  if(dataToSend != NULL) {
    LOG_TRACE((long) this << " sendErrorPacket called; a message send is in progress, so we need to finish it.");
    int count = this->peer().send_n(dataToSend + sendPosition, sendBufferSize - sendPosition);
    if(count >= 0) {
      sendPosition += count;
    }
    LOG_TRACE((long) this << " Sent remaining " << count << " bytes of current message (current postition " << sendPosition << "/" << sendBufferSize << ")");
    
    if(sendPosition >= (sendBufferSize)) {
      delete[] dataToSend;
      dataToSend = NULL;
      sendBufferSize = 0;
      sendPosition = 0;
    } else {
      LOG_ERROR((long) this << " Couldn't flush send buffer before sending error packet.");
    }
  }
  
  MessageHeader headerToSend;
  headerToSend.magicNumber = HEADER_MAGIC_NUMBER;
  headerToSend.size = 0;
  headerToSend.checksum = 0;
  headerToSend.priority = 127;
  headerToSend.error = errorCode;
  headerToSend.reserved[0] = 0;
  headerToSend.reserved[1] = 0;
  headerToSend.headerChecksum = ACE::crc32(&headerToSend, sizeof(headerToSend) - sizeof(headerToSend.headerChecksum));
  int count = this->peer().send_n(&headerToSend, sizeof(headerToSend));
  if(count != sizeof(headerToSend)) {
    LOG_WARN((long) this << " Unable to send full error packet.");
  }
}

ammo::protocol::MessageWrapper *AndroidServiceHandler::getNextMessageToSend() {
  ammo::protocol::MessageWrapper *msg = NULL;
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    msg = sendQueue.top().message;
    sendQueue.pop();
  }
  
  int size = sendQueue.size();
  sendQueueMutex.release();
  LOG_TRACE((long) this << " Dequeued a message to send.  " << size << " messages remain in queue.");
  
  return msg;
}

ammo::protocol::MessageWrapper *AndroidServiceHandler::getNextReceivedMessage() {
  ammo::protocol::MessageWrapper *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.top().message;
    receiveQueue.pop();
  }
  receiveQueueMutex.release();
  
  return msg;
}

void AndroidServiceHandler::addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority) {
  QueuedMessage queuedMsg;
  queuedMsg.priority = priority;
  queuedMsg.message = msg;
  
  if(priority != msg->message_priority()) {
    LOG_WARN((long) this << " Priority mismatch on received message: Header = " << (int) priority << ", Message = " << msg->message_priority());
  }
  
  receiveQueueMutex.acquire();
  queuedMsg.messageCount = receivedMessageCount;
  receivedMessageCount++;
  receiveQueue.push(queuedMsg);
  receiveQueueMutex.release();
}

AndroidServiceHandler::~AndroidServiceHandler() {
  LOG_TRACE((long) this << " In ~AndroidServiceHandler");
  delete messageProcessor;
}
