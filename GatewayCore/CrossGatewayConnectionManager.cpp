#include "CrossGatewayConnectionManager.h"
#include "CrossGatewayServiceHandler.h"
#include "GatewayConfigurationManager.h"
#include "log.h"

#include <ace/Connector.h>
#include <ace/Reactor.h>

const int SLEEP_TIME = 3;

CrossGatewayConnectionManager::CrossGatewayConnectionManager() : connector(NULL), handler(NULL), cancelMutex(), cancelled(false) {
  
}
  
int CrossGatewayConnectionManager::svc() {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  bool connected = false;
  int connectionAttempt = 1;
  handler = NULL;
  
  while(!connected && !isCancelled()) {
    LOG_INFO("Attempting connection...  attempt " << connectionAttempt);
    ACE_INET_Addr serverAddress(config->getCrossGatewayParentPort(), config->getCrossGatewayParentAddress().c_str());
    connector = new ACE_Connector<CrossGatewayServiceHandler, ACE_SOCK_Connector>();
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
      reactor->notify(this, ACE_Event_Handler::READ_MASK); //runs handle_input in the reactor's thread (so we don't have
                                                           //to worry about a bunch of synchronization junk).
    }
  }
  return 0;
}

int CrossGatewayConnectionManager::handle_input(ACE_HANDLE fd) {
  //handler->setParentConnector(gatewayConnector);
  //gatewayConnector->onConnect(connector, handler);
  return 0;
}

void CrossGatewayConnectionManager::cancel() {
  cancelMutex.acquire();
  cancelled = true;
  cancelMutex.release();
}

bool CrossGatewayConnectionManager::isCancelled() {
  cancelMutex.acquire();
  volatile bool ret = cancelled;
  cancelMutex.release();
  return ret;
}
