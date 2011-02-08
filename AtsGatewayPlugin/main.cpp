

#include <iostream>
#include <string>

#include "log.h"
#include "ace/Reactor.h"

#include "AtsHandler.h"
#include "GatewayConnector.h"

/**
  Data Interest is expressed when the service subscribes to messages of a type.
*/ 
int setRegisterDataInterest(GatewayConnector* gwc, std::string interest, AtsHandler* handler) {
  LOG_DEBUG("Registering data interest in..." << interest);
  gwc->registerDataInterest(interest, handler);
  return 0;
}

/**
  Pull Interest expresses the ability to service pull requests.
*/ 
int setRegisterPullInterest(GatewayConnector* gwc, std::string interest, AtsHandler* handler) {
  LOG_DEBUG("Registering pull interest in..." << interest);
  gwc->registerPullInterest(interest, handler);
  return 0;
}

int main(int argc, char **argv) {  
  std::cout << "Creating gateway connector..." << std::endl <<std:: flush;
  LOG_DEBUG("Creating gateway connector...");
  
  AtsHandler* dataHandler = new AtsHandler();
  
  GatewayConnector* gwc = new GatewayConnector(dataHandler);
  
  setRegisterPullInterest(gwc, RTC_PEOPLE_LIST_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_CREATE_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_LIST_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_ACTIVATE_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_PASSIVATE_NS, dataHandler);
  setRegisterDataInterest(gwc, RTC_CHANNEL_MEDIA_UPLOAD_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_LIST_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_FILE_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_MEDIA_CLIP_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_CHANNEL_AGGREGATE_NS, dataHandler);

  setRegisterDataInterest(gwc, RTC_INVITE_NS, dataHandler);
  setRegisterDataInterest(gwc, RTC_SHARE_GPS_NS, dataHandler);
  
  // Get the process-wide ACE_Reactor (i.e. the one with which the acceptor should have registered)
  ACE_Reactor* reactor = ACE_Reactor::instance();
  std::cout << "Starting event loop..." << std::endl << std::flush;
  reactor->run_reactor_event_loop();
}
