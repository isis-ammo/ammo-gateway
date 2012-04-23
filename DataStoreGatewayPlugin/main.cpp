#include <iostream>

#include "ace/Select_Reactor.h"
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

#include "UserSwitch.inl"
#include "LogConfig.inl"

#ifdef WIN32
#include "WinSvc.inl"
#endif

using namespace std;
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


class App
{
public:
  static App* instance();
  static void destroy();
private:
  static App* _instance;

private:
  App();
  ~App();

public:
  void init(int argc, char* argv[]);
  void run();
  void stop();

private:
  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor selectReactor;
  ACE_Reactor newReactor;
  auto_ptr<ACE_Reactor> delete_instance;

  ACE_Sig_Action no_sigpipe;
  ACE_Sig_Action original_action;

  SigintHandler* handleExit;

  DataStoreReceiver* receiver;
  GatewayConnector* gatewayConnector;
};

App* App::_instance = NULL;

App* App::instance()
{
  if (!_instance) {
    _instance = new App();
  }

  return _instance;
}

void App::destroy()
{
  delete _instance;
  _instance = NULL;
}

App::App() : newReactor(&selectReactor),
             delete_instance(ACE_Reactor::instance(&newReactor)),
			 no_sigpipe((ACE_SignalHandler) SIG_IGN),  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects during write)
			 handleExit(NULL),
			 receiver(NULL),
			 gatewayConnector(NULL)
{
}

App::~App()
{
  //if (this->receiver) {
  //  delete this->receiver;
  //}
  //if (this->gatewayConnector) {
  //  delete this->gatewayConnector;
  //}
}

void App::init(int argc, char* argv[])
{
  dropPrivileges();
  setupLogging("DataStoreGatewayPlugin");
  LOG_FATAL("=========");
  LOG_FATAL("AMMO Location Store Gateway Plugin ("
            << VERSION
            << " built on "
            << __DATE__
            << " at "
            << __TIME__
            << ")");

  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  no_sigpipe.register_action(SIGPIPE, &original_action);
  
  handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);

  LOG_DEBUG ("Creating location store receiver...");
  
  receiver = new DataStoreReceiver ();
  gatewayConnector = new GatewayConnector (receiver);

  DataStoreConfigManager *config =
	  DataStoreConfigManager::getInstance (receiver, gatewayConnector);
	  
  // Nothing further is done with 'config' since everything happens
  // in the constructor. This macro avoids the 'unused' warning.  
  ACE_UNUSED_ARG (config);

  // Make sure that the receiver and connector have been created and
  // passed to the config manager before calling this method.
  if (!receiver->init ()) {
    // Error msg already output, just exit w/o starting reactor.
    return;
  }
}

void App::run()
{
  ACE_Reactor *reactor = ACE_Reactor::instance ();
  LOG_DEBUG ("Starting event loop...");
  reactor->run_reactor_event_loop ();
}

void App::stop()
{
  ACE_Reactor::instance()->end_reactor_event_loop();
}

#ifdef WIN32

void SvcInit(DWORD argc, LPTSTR* argv)
{
  App::instance()->init(argc, argv);
}

void SvcRun()
{
  App::instance()->run();
}

void SvcStop()
{
  App::instance()->stop();
}

int main(int argc, char* argv[])
{
  const std::string svcName = "DataStoreGatewayPlugin";

  // Service installation command line option
  if (argc == 2) {
    if (lstrcmpi(argv[1], TEXT("install")) == 0) {
      try
      {
        WinSvc::install(svcName);
		return 0;
      }
      catch (WinSvcException e)
	  {
        cerr << e.what();
		return 1;
	  }
	}
  }

  // Normal service operation
  WinSvc::callbacks_t callbacks(SvcInit, SvcRun, SvcStop);

  App::instance();
  try
  {
    WinSvc::instance(svcName, callbacks);
    WinSvc::instance()->run();
  }
  catch (WinSvcException e)
  {
    LOG_FATAL(e.what());
	WinSvc::instance()->SvcReportEvent((char*) (e.what()));
  }
  catch (...) {
    WinSvc::instance()->SvcReportEvent("unknown exception");
  }

  App::destroy();

  return 0;
}
#else
int main(int argc, char** argv)
{
  App::instance()->init(argc, argv);
  App::instance()->run();
  App::instance()->destroy();
  return 0;
}
#endif
