/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */



#include <iostream>
#include <string>

#include "log.h"
#include "version.h"

#include "ace/Select_Reactor.h"
#include "ace/Reactor.h"

#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 

#include "AtsHandler.h"
#include "GatewayConnector.h"

#include "UserSwitch.inl"
#include "LogConfig.inl"

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
  dropPrivileges();
  
  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor selectReactor;
  ACE_Reactor newReactor(&selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(&newReactor));
  
  setupLogging("AtsGatewayPlugin");
  LOG_FATAL("=========");
  LOG_FATAL("AMMO Ats Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
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
