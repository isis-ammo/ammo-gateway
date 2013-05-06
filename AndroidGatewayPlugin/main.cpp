#include <iostream>
#include <string>
#include <queue>

#include "ace/INET_Addr.h"
#include "ace/SOCK_Connector.h"
#include "ace/SOCK_Stream.h"
#include "ace/SOCK_Acceptor.h"

#include "ace/OS_NS_unistd.h"
#include "ace/Signal.h"

#include "ace/Acceptor.h"
#include "ace/Reactor.h"
#include "ace/Select_Reactor.h"

#include "log.h"
#include "version.h"

#include "UserSwitch.inl"
#include "LogConfig.inl"

#ifdef WIN32
#include "WinSvc.inl"
#endif

#include "NetworkAcceptor.h"
#include "AndroidEventHandler.h"
#include "NetworkEnumerations.h"

#include "AndroidPluginConfigurationManager.h"
#include "ConnectionManager.h"

using namespace std;
using namespace ammo::gateway::internal;

string gatewayAddress;
int gatewayPort;

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
  bool init(int argc, char* argv[]);
  void run();
  void stop();

private:
  ACE_Sig_Action no_sigpipe;
  ACE_Sig_Action original_action;

  SigintHandler* handleExit;

  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor for accept events
  NetworkAcceptor<ammo::protocol::MessageWrapper, AndroidEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0xfeedbeef>* acceptor;
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

App::App() : no_sigpipe((ACE_SignalHandler) SIG_IGN),  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects during write)
			 handleExit(NULL),
			 acceptor(NULL)
{
}

App::~App()
{
  if (this->acceptor) {
    delete this->acceptor;
  }
}

bool App::init(int argc, char* argv[])
{
  dropPrivileges();
  

  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor *selectReactor = new ACE_Select_Reactor;
  ACE_Reactor *newReactor = new ACE_Reactor(selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(newReactor));
  
  setupLogging("AndroidGatewayPlugin");
  LOG_FATAL("=========");
  LOG_FATAL("AMMO Android Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  no_sigpipe.register_action(SIGPIPE, &original_action);
  
  handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);

  string androidAddress = "0.0.0.0";
  int androidPort = 33289;
  
  queue<string> argumentQueue;
  for(int i=1; i < argc; i++) {
    argumentQueue.push(string(argv[i]));
  }
  
  while(!argumentQueue.empty()) {
    string arg = argumentQueue.front();
    argumentQueue.pop();
    
    if(arg == "--listenPort" && argumentQueue.size() >= 1) {
      string param = argumentQueue.front();
      argumentQueue.pop();
      androidPort = atoi(param.c_str());
    } else if(arg == "--listenAddress" && argumentQueue.size() >= 1) {
      string param = argumentQueue.front();
      argumentQueue.pop();
      androidAddress = param;
    } else if(arg == "-nowinsvc") {
      //don't consider -nowinsvc an error (functionality is in main())
    } else {
      LOG_FATAL("Usage: AndroidGatewayPlguin [--listenPort port] [--listenAddress address]");
      LOG_FATAL("  --listenPort port        Sets the listening port for the Android ");
      LOG_FATAL("                           interface (default 33289)");
      LOG_FATAL("  --listenAddress address  Sets the listening address for the Android");
      LOG_FATAL("                           interface (default 0.0.0.0, or all interfaces)");
      return false;
    }
  }
  
  //Create config manager here to avoid parsing later and to avoid threading
  //issues
  AndroidPluginConfigurationManager::getInstance();
  
  LOG_DEBUG("Creating acceptor...");
  
  //Creates and opens the socket acceptor; registers with the singleton ACE_Reactor
  //for accept events
  acceptor = new NetworkAcceptor<ammo::protocol::MessageWrapper, 
                                 AndroidEventHandler, 
								 ammo::gateway::internal::SYNC_MULTITHREADED, 
								 0xfeedbeef>(androidAddress, 
								             androidPort);

  LOG_INFO("Listening on port " << androidPort << " on interface " << androidAddress);
  
  LOG_DEBUG("Creating connection manager...");
  ConnectionManager *connManager = ConnectionManager::getInstance();
  
  ACE_Reactor::instance()->schedule_timer(connManager, NULL, ACE_Time_Value(ConnectionManager::TIMEOUT_TIME_SECONDS));

  return true;
}

void App::run()
{
  //Get the process-wide ACE_Reactor (the one the acceptor should have registered with)
  ACE_Reactor *reactor = ACE_Reactor::instance();
  LOG_DEBUG("Starting event loop...");
  reactor->run_reactor_event_loop();
  LOG_DEBUG("Event loop terminated.");
}

void App::stop()
{
  ACE_Reactor::instance()->end_reactor_event_loop();
}

#ifdef WIN32

bool SvcInit(DWORD argc, LPTSTR* argv)
{
  return App::instance()->init(argc, argv);
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
  const std::string svcName = "AndroidGatewayPlugin";

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
	else if (lstrcmpi(argv[1], TEXT("-nowinsvc")) == 0) {
      if (!App::instance()->init(argc, argv)) {
        return 1;
      }
      App::instance()->run();
      App::instance()->destroy();
      return 0;
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
  }

  App::destroy();

  return 0;
}
#else
int main(int argc, char** argv)
{
  if (!App::instance()->init(argc, argv)) {
    return 1;
  }
  App::instance()->run();
  App::instance()->destroy();
  return 0;
}
#endif
