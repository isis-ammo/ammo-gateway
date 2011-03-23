#ifndef GATEWAY_CONNECTION_MANAGER_H
#define GATEWAY_CONNECTION_MANAGER_H

#include <ace/Task.h>
#include <ace/Connector.h>
#include <ace/SOCK_Connector.h>

class GatewayConnector;
class GatewayServiceHandler;

class GatewayConnectionManager : public ACE_Task <ACE_MT_SYNCH> {
public:
  GatewayConnectionManager(GatewayConnector *connector);
  
  virtual int svc();  
  virtual int handle_input(ACE_HANDLE fd);
  
private:
  GatewayConnector *gatewayConnector;
  
  ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector> *connector;
  GatewayServiceHandler *handler;
};

#endif //GATEWAY_CONNECTION_MANAGER_H
