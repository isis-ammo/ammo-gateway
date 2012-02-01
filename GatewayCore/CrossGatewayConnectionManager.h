#ifndef GATEWAY_CONNECTION_MANAGER_H
#define GATEWAY_CONNECTION_MANAGER_H

#include <ace/Task.h>

#include "NetworkConnector.h"
#include "CrossGatewayEventHandler.h"
#include "NetworkEnumerations.h"
#include "protocol/GatewayPrivateMessages.pb.h"

class CrossGatewayEventHandler;

class CrossGatewayConnectionManager : public ACE_Task <ACE_MT_SYNCH> {
public:
  CrossGatewayConnectionManager();
  
  virtual int svc();  
  virtual int handle_input(ACE_HANDLE fd);
  
  void cancel();
  bool isCancelled();
  
private:        
  ammo::gateway::internal::NetworkConnector<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d> *connector;
  CrossGatewayEventHandler *handler;
  
  ACE_Thread_Mutex cancelMutex;
  bool cancelled;
};

#endif //GATEWAY_CONNECTION_MANAGER_H
