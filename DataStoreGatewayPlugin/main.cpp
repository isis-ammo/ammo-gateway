#include "ace/Reactor.h"
#include "ace/OS_NS_unistd.h" 
#include "ace/Signal.h" 
#include "ace/Time_Value.h"
#include "ace/OS_NS_time.h"
#include "ace/Date_Time.h"

#include "log.h"
#include "version.h"

#include "DataStoreReceiver.h"
#include "DataStoreConfigManager.h"

using namespace ammo::gateway;

//Handle SIGINT so the program can exit cleanly (otherwise, we just terminate
//in the middle of the reactor event loop, which isn't always a good thing).
class SigintHandler : public ACE_Event_Handler {
public:
  int
  handle_signal (int signum, siginfo_t * = 0, ucontext_t * = 0)
  {
    if (signum == SIGINT || signum == SIGTERM)
      {
        ACE_Reactor::instance ()->end_reactor_event_loop ();
      }
    
    return 0;
  }
};

int main (int /* argc */, char ** /* argv */)
{
  LOG_INFO ("AMMO Location Store Gateway Plugin ("
            << VERSION
            << " built on "
            << __DATE__
            << " at "
            << __TIME__
            << ")");
  
  SigintHandler * handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);
  
  LOG_DEBUG ("Creating location store receiver...");
  
  DataStoreReceiver *receiver = new DataStoreReceiver ();

  GatewayConnector *gatewayConnector = new GatewayConnector (receiver);

  DataStoreConfigManager *config =
	  DataStoreConfigManager::getInstance (receiver, gatewayConnector);
	  
	// Nothing further is done with 'config' since everything happens
	// in the constructor. This macro avoids the 'unused' warning.  
	ACE_UNUSED_ARG (config);

	if (!receiver->init ())
	  {
	    // Error msg already output, just exit w/o starting reactor.
	    return -1;
	  }
	  
//====================================
/*
  std::string mime_t ("application/vnd.edu.vu.isis.ammo.private_contacts");
//  std::string mime_t ("application/vnd.edu.vu.isis.ammo.dash.media");
  std::string orig_user ("kyle.anderson");
  std::string uri ("kakama");
  std::string data ("{\"first_name\":\"Jimmy\",\"middle_initial\":\"I\",\"last_name\":\"Bork\",\"rank\":\"sgt\",\"call_sign\":\"\",\"branch\":\"\",\"unit\":\"\",\"email\":\"\",\"phone\":\"\"}");
//  std::string data ("{\"eventId\":\"booga\",\"dataType\":\"POD\",\"data\":\"xxx\",\"createdDate\":0,\"modifiedDate\":0}");
  
  ammo::gateway::PushData pd;
  pd.mimeType = mime_t;
  pd.originUsername = orig_user;
  pd.uri = uri;
  pd.data = data;

  receiver->onPushDataReceived (0, pd);


  std::string requestUid ("requestUid");
  std::string pluginId ("pluginId");
  std::string query ("kyle.anderson,kakama,Jimmy,,,,,,,,");
//  std::string query ("kakama,,,,");
//  std::string projection (",POD,,,,,");
  std::string projection ("");
  
  ammo::gateway::PullRequest pr;
  pr.requestUid = requestUid;
  pr.pluginId = pluginId;
  pr.mimeType = mime_t;
  pr.query = query;
  pr.projection = projection;
  pr.maxResults = 0;
  pr.startFromCount = 0;
  pr.liveQuery = false;
  
  receiver->onPullRequestReceived (0, pr);

  delete receiver;
  */
//===================================

  // Get the process-wide ACE_Reactor (the one the acceptor should
  // have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance ();
  LOG_DEBUG ("Starting event loop...");
  reactor->run_reactor_event_loop ();

  return 0;
}
