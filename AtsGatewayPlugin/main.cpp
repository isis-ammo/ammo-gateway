

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
  AtsConfigMgr* config = AtsConfigMgr::getInstance(); // load the configuration file

  LOG_DEBUG("Creating gateway connector...");
  LOG_INFO(" host: "+ config->getHost());
  LOG_INFO(" dir:  "+ config->getBaseDir());
  LOG_INFO(" url:  "+ config->getUrl());
  
  AtsHandler* dataHandler = new AtsHandler();
  
  GatewayConnector* gwc = (config->hasGatewayConfig())
         ? new GatewayConnector(dataHandler, config->getGatewayConfig())
         : new GatewayConnector(dataHandler);
  
  setRegisterPullInterest(gwc, RTC_LIST_PEOPLE_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_LIST_CHANNEL_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_ACTIVATE_CHANNEL_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_PASSIVATE_CHANNEL_NS, dataHandler);

  setRegisterPullInterest(gwc, RTC_LIST_CHANNEL_MEDIA_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_GET_CHANNEL_MEDIA_FILE_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_GET_CHANNEL_MEDIA_CLIP_NS, dataHandler);
  setRegisterPullInterest(gwc, RTC_AGGREGATE_CHANNEL_NS, dataHandler);

  setRegisterDataInterest(gwc, RTC_CREATE_CHANNEL_NS, dataHandler);
  setRegisterDataInterest(gwc, RTC_UPLOAD_CHANNEL_MEDIA_NS, dataHandler);

  setRegisterDataInterest(gwc, RTC_INVITE_NS, dataHandler);
  setRegisterDataInterest(gwc, RTC_SHARE_GPS_NS, dataHandler);
  
  // Get the process-wide ACE_Reactor (i.e. the one with which the acceptor should have registered)
  ACE_Reactor* reactor = ACE_Reactor::instance();
  std::cout << "Starting event loop..." << std::endl << std::flush;
  reactor->run_reactor_event_loop();

  return 0;
}
