#ifndef NETWORK_SERVICE_HANDLER_H
#define NETWORK_SERVICE_HANDLER_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/OS_NS_errno.h"
#include <vector>
#include <queue>

#include "log.h"

#include "NetworkEnumerations.h"
#include "NetworkEventHandler.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      struct MessageHeader {
        unsigned int magicNumber;    //Always set to MagicNumber (template parameter)
        unsigned int size;           //size of the data (does *not* include header)
        char         priority;       //Message priority (larger numbers are higher priority, and will be processed first if messages are queued)
        char         error;          //Error code (if nonzero, size and message checksum should be zero)
        char         reserved[2];    //Reserved for future use, and to ensure header word alignment
        unsigned int checksum;       //CRC32 checksum of the data (does *not* include header)
        unsigned int headerChecksum; //CRC32 checksum of the header, less this checksum.  Does *not* include data, or itself.
      };
      
      //error values for MessageHeader
      const char NO_MESSAGE_ERROR = 0;
      const char INVALID_MAGIC_NUMBER = 1;
      const char INVALID_HEADER_CHECKSUM = 2;
      const char INVALID_MESSAGE_CHECKSUM = 3;
      const char MESSAGE_TOO_LARGE = 4;
      
      template <class ProtobufMessageWrapper>
      struct QueuedMessage {
        char priority;
        unsigned long long messageCount; //messages of the same priority should come out in first-in first-out order (STL queue doesn't guarantee this)
        ProtobufMessageWrapper *message;
      };
      
      template <class ProtobufMessageWrapper>
      class QueuedMessageComparison {
      public:
        bool operator()(QueuedMessage<ProtobufMessageWrapper> &first, QueuedMessage<ProtobufMessageWrapper> &second) { //returns true if first is lower priority than second
          if(first.priority < second.priority) {
            return true;
          } else if(first.priority == second.priority && first.messageCount > second.messageCount) {
            return true;
          } else {
            return false;
          }
        }
      };
      
      template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
      public:
        NetworkServiceHandler();
        
        int open(void *ptr = 0);
        
        int handle_close(ACE_HANDLE fd = ACE_INVALID_HANDLE, ACE_Reactor_Mask=ACE_Event_Handler::ALL_EVENTS_MASK);
        int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
        int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
        
        int processData(char *collectedData, unsigned int dataSize, unsigned int checksum, char priority);
        
        void sendMessage(ProtobufMessageWrapper *msg, char priority);
        ProtobufMessageWrapper *getNextMessageToSend();
        
        NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber> *eventHandler;
        
        ~NetworkServiceHandler();
        
      protected:
        void sendErrorPacket(char errorCode);
        
        typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
        
        typedef enum {
          READING_HEADER = 0,
          READING_DATA = 1
        } ReaderState;
       
        ReaderState state;
        MessageHeader messageHeader;
        char *collectedData;
        unsigned int position;
        
        char *dataToSend;
        unsigned int sendPosition;
        unsigned int sendBufferSize;
        
        bool connectionClosing;
        
        std::string deviceId; //not validated; just for pretty logging
        
        ACE_Thread_Mutex sendQueueMutex;
        ACE_Thread_Mutex receiveQueueMutex;
        
        typedef std::priority_queue<QueuedMessage<ProtobufMessageWrapper>, std::vector<QueuedMessage<ProtobufMessageWrapper> >, QueuedMessageComparison<ProtobufMessageWrapper> > MessageQueue;
        MessageQueue sendQueue;
        MessageQueue receiveQueue;
        
        unsigned long long sentMessageCount;
        unsigned long long receivedMessageCount;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkServiceHandler() : 
eventHandler(NULL),
sendQueueMutex(), 
receiveQueueMutex()
{
  LOG_TRACE("In NetworkServiceHandler() ctor");
  eventHandler = static_cast<NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber> *>(new EventHandler());
  eventHandler->setServiceHandler(this);
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::open(void *ptr) {
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
  std::string remoteAddressAsString;
  int result = this->peer().get_remote_addr(remoteAddress);
  if(result == 0) {
    remoteAddressAsString = remoteAddress.get_host_addr();
    LOG_INFO((long) this << " Got connection from " << remoteAddressAsString);
  } else {
    LOG_WARN((long) this << " Got new connection, but couldn't determine remote address.");
  }
  
  this->peer().enable(ACE_NONBLOCK);
  
  this->eventHandler->onConnect(remoteAddressAsString);
  
  return 0;
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::handle_close(ACE_HANDLE fd, ACE_Reactor_Mask m) {
  //TODO: send event handler its close event here
  this->eventHandler->onDisconnect();
  int result = super::handle_close(fd, m);
  
  return result;
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::handle_input(ACE_HANDLE fd) {
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
        if(messageHeader.magicNumber == MagicNumber) {
          unsigned int calculatedChecksum = ACE::crc32(&messageHeader, sizeof(messageHeader) - sizeof(messageHeader.headerChecksum));
          if(calculatedChecksum != messageHeader.headerChecksum) {
            LOG_ERROR((long) this << " Invalid header checksum");
            sendErrorPacket(INVALID_HEADER_CHECKSUM);
            return -1;
          }
        } else {
          LOG_ERROR((long) this << " Invalid magic number: " << std::hex << messageHeader.magicNumber << std::dec);
          sendErrorPacket(INVALID_MAGIC_NUMBER);
          return -1;
        }
        
        try {
          collectedData = new char[messageHeader.size];
        } catch (std::bad_alloc) {
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

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::handle_output(ACE_HANDLE fd) {
  int count = 0;
  
  do {
    if(dataToSend == NULL) {
      ProtobufMessageWrapper *msg = getNextMessageToSend();
      if(msg != NULL) {
        LOG_TRACE((long) this << " Getting a new message to send");
        if(!msg->IsInitialized()) {
          LOG_WARN((long) this << " Protocol Buffers message is missing a required element.");
        }
        unsigned int messageSize = msg->ByteSize();
        sendBufferSize = messageSize + sizeof(MessageHeader);
        dataToSend = new char[sendBufferSize];
        ammo::gateway::internal::MessageHeader *headerToSend = (ammo::gateway::internal::MessageHeader *) dataToSend;
        headerToSend->magicNumber = MagicNumber;
        headerToSend->size = messageSize;
        headerToSend->priority = msg->message_priority();
        headerToSend->error = NO_MESSAGE_ERROR;
        headerToSend->reserved[0] = 0;
        headerToSend->reserved[1] = 0;
        
        char *protobufSerializedMessage = dataToSend + sizeof(ammo::gateway::internal::MessageHeader);
        msg->SerializeToArray(protobufSerializedMessage, messageSize);
        
        headerToSend->checksum = ACE::crc32(protobufSerializedMessage, messageSize);
        headerToSend->headerChecksum = ACE::crc32(headerToSend, sizeof(messageHeader) - sizeof(headerToSend->headerChecksum));
        
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
    LOG_TRACE((long) this << " Sent " << count << " bytes (current position " << sendPosition << "/" << sendBufferSize);
    
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

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::processData(char *data, unsigned int messageSize, unsigned int messageChecksum, char priority) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if(calculatedChecksum != messageChecksum) {
    LOG_ERROR((long) this << " Mismatched checksum " << std::hex << calculatedChecksum << " : " << messageChecksum);
    LOG_ERROR((long) this << " size " << std::dec << messageSize ); // << " payload: " < );
    //TODO: Call event handler's onError method
    return -1;
  }
  
  //checksum is valid; parse the data
  ProtobufMessageWrapper *msg = new ProtobufMessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR((long) this << " MessageWrapper could not be deserialized.");
    LOG_ERROR((long) this << " Client must have sent something that isn't a protocol buffer (or the wrong type).");
    delete msg;
    //TODO: Call event handler's onError method
    return -1;
  }
  
  this->eventHandler->onMessageAvailable(msg);
  
  return 0;
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::sendMessage(ProtobufMessageWrapper *msg, char priority) {
  QueuedMessage<ProtobufMessageWrapper> queuedMsg;
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

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::sendErrorPacket(char errorCode) {
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
  headerToSend.magicNumber = MagicNumber;
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

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ProtobufMessageWrapper *ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::getNextMessageToSend() {
  ProtobufMessageWrapper *msg = NULL;
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

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkServiceHandler() {
  LOG_TRACE((long) this << " In ~NetworkServiceHandler");
  delete eventHandler;
}

#endif
