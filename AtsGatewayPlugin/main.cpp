#include <iostream>
#include <string>

#include "ace/Reactor.h"

#include "GatewayConnector.h"
#include "AtsHandler.h"

int main(int argc, char **argv) {  
  std::cout << "Creating gateway connector..." << std::endl <<std:: flush;
  
  AtsHandler *dataHandler = new AtsHandler();
  
  GatewayConnector *gatewayConnector = new GatewayConnector(dataHandler);
  
  gatewayConnector->registerDataInterest("text/plain", dataHandler);
  
  for(;;) {
    gatewayConnector->pushData("someUrl", "sometype", "someData");
  }
  
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  std::cout << "Starting event loop..." << std::endl << std::flush;
  reactor->run_reactor_event_loop();
}
