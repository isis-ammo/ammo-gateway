#ifndef GATEWAY_CONNECTION_MANAGER_H
#define GATEWAY_CONNECTION_MANAGER_H

#include <ace/Task.h>
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>

class CrossGatewayServiceHandler;

class CrossGatewayConnectionManager : public ACE_Task <ACE_MT_SYNCH> {
public:
  CrossGatewayConnectionManager();
  
  virtual int svc();  
  virtual int handle_input(ACE_HANDLE fd);
  
  void cancel();
  bool isCancelled();
  
private:        
  ACE_Connector<CrossGatewayServiceHandler, ACE_SOCK_Connector> *connector;
  CrossGatewayServiceHandler *handler;
  
  ACE_Thread_Mutex cancelMutex;
  bool cancelled;
};

#endif //GATEWAY_CONNECTION_MANAGER_H
