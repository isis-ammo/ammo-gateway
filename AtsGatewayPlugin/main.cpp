

#include <iostream>
#include <string>

#include "log.h"
#include "version.h"

#include "ace/Reactor.h"

#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 

#include "AtsHandler.h"
#include "GatewayConnector.h"

#include "UserSwitch.inl"

using namespace ammo::gateway;

//Handle SIGINT so the program can exit cleanly (otherwise, we just terminate
//in the middle of the reactor event loop, which isn't always a good thing).
class SigintHandler : public ACE_Event_Handler {
public:
  int handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0) {
    if (signum == SIGINT || signum == SIGTERM) {
      ACE_Reactor::instance()->end_reactor_event_loop();
    }
    return 0;
  }
};

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
  LOG_INFO("AMMO Ats Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  dropPrivileges();
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
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
  setRegisterPullInterest(gwc, PLI_LIST_UNIT_NS, dataHandler);
  setRegisterPullInterest(gwc, PLI_LIST_LOC_NS, dataHandler);
  setRegisterPullInterest(gwc, PLI_MEMBERS_NS, dataHandler);

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
  
  setRegisterDataInterest(gwc, PLI_POST_LOC_NS, dataHandler); // Scope: LOCAL - the subscribe API does not have a scope parameter currently
  setRegisterDataInterest(gwc, PLI_POST_LOCS_NS, dataHandler); // Scope: GLOBAL 


  // Get the process-wide ACE_Reactor (i.e. the one with which the acceptor should have registered)
  ACE_Reactor* reactor = ACE_Reactor::instance();
  std::cout << "Starting event loop..." << std::endl << std::flush;
  reactor->run_reactor_event_loop();

  return 0;
}
