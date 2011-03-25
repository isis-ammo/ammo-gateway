#include "GatewayConnectionManager.h"
#include "GatewayConnector.h"
#include "GatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "log.h"

#include <ace/Connector.h>
#include <ace/Reactor.h>

const int SLEEP_TIME = 3;

GatewayConnectionManager::GatewayConnectionManager(GatewayConnector *connector) : gatewayConnector(connector), connector(NULL), handler(NULL), cancelMutex(), cancelled(false) {
  
}
  
int GatewayConnectionManager::svc() {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  bool connected = false;
  int connectionAttempt = 1;
  handler = NULL;
  
  while(!connected && !isCancelled()) {
    LOG_INFO("Attempting connection...  attempt " << connectionAttempt);
    ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayAddress().c_str());
    connector = new ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector>();
    int status = connector->connect(handler, serverAddress);
    if(status == -1) {
      LOG_ERROR("Connection to gateway failed (" << errno << ": " << strerror(errno) << ")" );
      connectionAttempt++;
      connected = false;
      delete connector;
      connector = NULL;
      handler = NULL;
      ACE_OS::sleep(SLEEP_TIME);
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

void GatewayConnectionManager::cancel() {
  cancelMutex.acquire();
  cancelled = true;
  cancelMutex.release();
}

bool GatewayConnectionManager::isCancelled() {
  cancelMutex.acquire();
  volatile bool ret = cancelled;
  cancelMutex.release();
  return ret;
}
