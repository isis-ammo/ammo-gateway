#ifndef ANDROID_SERVICE_HANDLER_H
#define ANDROID_SERVICE_HANDLER_H

#include "GatewayConnector.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/AmmoMessages.pb.h"
#include <vector>
#include <queue>

class AndroidMessageProcessor;

const unsigned int HEADER_MAGIC_NUMBER = 0xfeedbeef;

struct MessageHeader {
  unsigned int magicNumber;    //Always set to 0xfeedbeef
  unsigned int size;           //size of the data (does *not* include header)
  char         priority;       //Message priority (larger numbers are higher priority, and will be processed first if messages are queued)
  char         reserved[3];    //Reserved for future use, and to ensure header word alignment
  unsigned int checksum;       //CRC32 checksum of the data (does *not* include header)
  unsigned int headerChecksum; //CRC32 checksum of the header, less this checksum.  Does *not* include data, or itself.
};

struct QueuedMessage {
  char priority;
  ammo::protocol::MessageWrapper *message;
};

class AndroidServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>{
public:
  AndroidServiceHandler();
  
  int open(void *ptr = 0);
  
  int handle_close(ACE_HANDLE fd = ACE_INVALID_HANDLE, ACE_Reactor_Mask=ACE_Event_Handler::ALL_EVENTS_MASK);
  
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum, char priority);
  
  void sendMessage(ammo::protocol::MessageWrapper *msg, char priority);
  ammo::protocol::MessageWrapper *getNextMessageToSend();
  
  ammo::protocol::MessageWrapper *getNextReceivedMessage();
  void addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority);
  
  ~AndroidServiceHandler();
  
protected:
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
  
  AndroidMessageProcessor *messageProcessor;
  ACE_Thread_Mutex sendQueueMutex;
  ACE_Thread_Mutex receiveQueueMutex;
  
  typedef std::queue<QueuedMessage> MessageQueue;
  MessageQueue sendQueue;
  MessageQueue receiveQueue;
};

#endif        //  #ifndef ANDROID_SERVICE_HANDLER_H

