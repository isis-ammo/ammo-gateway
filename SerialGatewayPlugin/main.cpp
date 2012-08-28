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
#include "ace/Select_Reactor.h"
#include "ace/Reactor.h"

#include "GpsThread.h"
#include "SerialServiceHandler.h"
#include "SerialConfigurationManager.h"

#include "log.h"
#include "version.h"

#include "UserSwitch.inl"
#include "LogConfig.inl"

#ifdef WIN32
#include "WinSvc.inl"
#endif

using namespace std;

string gatewayAddress;
int gatewayPort;

extern int  serial_receiver( int argc, char *argv[], void ( *process_message) ( int sender, int size, char *full_buffer ) );



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


extern void testParseTerse();

ACE_THR_FUNC_RETURN start_svc_handler( void *data ) {
  LOG_DEBUG("Receiving message receiver - blocking call ");
  SerialServiceHandler *svcHandler = static_cast<SerialServiceHandler *>(data);
  svcHandler->receiveData();
  return (ACE_THR_FUNC_RETURN) 0;
}


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

  SerialServiceHandler* svcHandler;
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
			 svcHandler(NULL)
{
}

App::~App()
{
}

bool App::init(int argc, char* argv[])
{
  dropPrivileges();
  setupLogging("SerialGatewayPlugin");
  LOG_FATAL("=========");
  LOG_FATAL("AMMO Serial Gateway Plugin (" << VERSION << " built on " << __DATE__ << " at " << __TIME__ << ")");
  
  //Explicitly specify the ACE select reactor; on Windows, ACE defaults
  //to the WFMO reactor, which has radically different semantics and
  //violates assumptions we made in our code
  ACE_Select_Reactor *selectReactor = new ACE_Select_Reactor;
  ACE_Reactor *newReactor = new ACE_Reactor(selectReactor);
  auto_ptr<ACE_Reactor> delete_instance(ACE_Reactor::instance(newReactor));
  
  // Set signal handler for SIGPIPE (so we don't crash if a device disconnects
  // during write)
  no_sigpipe.register_action(SIGPIPE, &original_action);

  handleExit = new SigintHandler();
  ACE_Reactor::instance()->register_handler(SIGINT, handleExit);
  ACE_Reactor::instance()->register_handler(SIGTERM, handleExit);

  SerialConfigurationManager* config = SerialConfigurationManager::getInstance();
  
  GpsThread *gpsThread = new GpsThread();
  gpsThread->activate(THR_NEW_LWP | THR_JOINABLE, 1, 1, ACE_THR_PRI_FIFO_DEF);

  LOG_DEBUG("Creating service handler which receives and routes to gateway via the GatewayConnector");
  //svcHandler = new SerialServiceHandler();
  //svcHandler->open( (void *)(config->getListenPort().c_str()) );

  //ACE_Thread::spawn( &start_svc_handler, (void *)svcHandler  );

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
  const std::string svcName = "SerialGatewayPlugin";

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
