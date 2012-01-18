#ifndef NETWORK_SERVICE_HANDLER_H
#define NETWORK_SERVICE_HANDLER_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include <vector>
#include <queue>

namespace ammo {
  namespace gateway {
    namespace internal {
      struct MessageHeader {
        unsigned int magicNumber;    //Always set to 0xfeedbeef
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
      
      template <class ProtobufMessageWrapper, class EventHandler, SynchronizationMethod SyncMethod, int MagicNumber>
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
        
        ProtobufMessageWrapper *getNextReceivedMessage();
        void addReceivedMessage(ProtobufMessageWrapper *msg, char priority);
        
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

#endif
