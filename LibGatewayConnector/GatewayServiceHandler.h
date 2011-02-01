#ifndef GATEWAY_SERVICE_HANDLER_H
#define GATEWAY_SERVICE_HANDLER_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <vector>
#include <queue>

class GatewayConnector;
class GatewayMessageProcessor;

class GatewayServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
public:
  //GatewayServiceHandler(ACE_Thread_Manager *tm, ACE_Message_Queue<ACE_NULL_SYNCH> *mq, ACE_Reactor *reactor);
  GatewayServiceHandler();
  virtual ~GatewayServiceHandler();
  int open(void *ptr = 0);
  
  int handle_close(ACE_HANDLE fd = ACE_INVALID_HANDLE, ACE_Reactor_Mask=ACE_Event_Handler::ALL_EVENTS_MASK);
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum);
  
  void sendMessage(ammo::gateway::protocol::GatewayWrapper *msg);
  ammo::gateway::protocol::GatewayWrapper *getNextMessageToSend();
  
  ammo::gateway::protocol::GatewayWrapper *getNextReceivedMessage();
  void addReceivedMessage(ammo::gateway::protocol::GatewayWrapper *msg);
  
  void setParentConnector(GatewayConnector *parent);
  
protected:
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
  
  typedef enum {
    READING_SIZE = 0,
    READING_CHECKSUM = 1,
    READING_DATA = 2
  } ReaderState;
  
  ReaderState state;
  unsigned int dataSize;
  unsigned int checksum;
  char *collectedData;
  unsigned int position;
  
  char *dataToSend;
  unsigned int sendPosition;
  unsigned int sendBufferSize;
  
  GatewayConnector *parent;
  GatewayMessageProcessor *messageProcessor;
  
  ACE_Thread_Mutex sendQueueMutex;
  ACE_Thread_Mutex receiveQueueMutex;
  
  std::queue<ammo::gateway::protocol::GatewayWrapper *> sendQueue;
  std::queue<ammo::gateway::protocol::GatewayWrapper *> receiveQueue;
};

#endif        //  #ifndef GATEWAY_SERVICE_HANDLER_H

