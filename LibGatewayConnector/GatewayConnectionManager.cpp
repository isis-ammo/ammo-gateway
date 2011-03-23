#include "GatewayConnectionManager.h"
#include "GatewayConnector.h"
#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "log.h"

#include <ace/Connector.h>
#include <ace/Reactor.h>

GatewayConnectionManager::GatewayConnectionManager(GatewayConnector *connector) : gatewayConnector(connector), connector(NULL), handler(NULL) {
  
}
  
int GatewayConnectionManager::svc() {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  bool connected = false;
  int connectionAttempt = 1;
  handler = NULL;
  
  while(!connected) {
    LOG_INFO("Attempting connection...  attempt " << connectionAttempt);
    ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayAddress().c_str());
    connector = new ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector>();
    int status = connector->connect(handler, serverAddress);
    if(status == -1) {
      LOG_ERROR("connection to gateway failed");
      LOG_ERROR("errno: " << errno);
      LOG_ERROR("error: " << strerror(errno));
      connectionAttempt++;
      connected = false;
    } else {
      LOG_INFO("Connection to gateway established.");
      connected = true;
      ACE_Reactor *reactor = ACE_Reactor::instance();
      reactor->notify(this, ACE_Event_Handler::READ_MASK);
    }
  }
  return 0;
}

int GatewayConnectionManager::handle_input(ACE_HANDLE fd) {
  handler->setParentConnector(gatewayConnector);
  gatewayConnector->onConnect(connector, handler);
  return 0;
}
