#ifndef Gateway_MESSAGE_PROCESSOR_H
#define Gateway_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include "GatewayConnector.h"

class GatewayServiceHandler;

class GatewayMessageProcessor : public ACE_Task <ACE_MT_SYNCH> {
public:
  GatewayMessageProcessor(GatewayServiceHandler *serviceHandler);
  virtual ~GatewayMessageProcessor();
  
  void setParentConnector(GatewayConnector *parent);
  
  virtual int open(void *args);
  
  virtual int close();
  
  virtual int svc();
  
  void signalNewMessageAvailable();
  
private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  GatewayServiceHandler *commsHandler;
  GatewayConnector *parent;
  
  bool isClosed();
  void processMessage(ammo::gateway::protocol::GatewayWrapper &msg);
};

#endif
