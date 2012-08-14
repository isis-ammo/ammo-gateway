/**
 * Windows Service
 *
 * Based on the Complete Service Sample from MSDN
 * http://msdn.microsoft.com/en-us/library/windows/desktop/bb540476%28v=vs.85%29.aspx
 */

#include <stdexcept>
#include <string>
#include <sstream>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#pragma comment(lib, "advapi32.lib")
#pragma warning(disable: 4290)

/**
 * Exception class for WinSvc
 */
class WinSvcException : public std::exception
{
public:
  WinSvcException(const std::string& msg, DWORD errorCode);
  ~WinSvcException() {}
  const char* what() const throw() { return this->msg.c_str(); }

private:
  std::string msg;
};

WinSvcException::WinSvcException(const std::string& msg,
                                 DWORD errorCode)
{
  char* winMsg = NULL;

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPSTR) &winMsg,
                0,
                NULL);

  std::stringstream ss;
  ss << msg << ": (" << errorCode << ") ";
  ss << ((winMsg) ? winMsg : "Failed to get error message");

  this->msg = ss.str();

  LocalFree(winMsg);
}

/** Windows service callback for initialization of the service */
typedef bool (*WinSvcInitCB)(DWORD argc, LPTSTR* argv);
/** Windows service callback to perform the service's work */
typedef void (*WinSvcRunCB)();
/** Windows service callback to stop the service */
typedef void (*WinSvcStopCB)();

/**
 * The WinSvc class contains both code to run as a Windows Service as well as
 * utilities to install, delete, config, etc services.
 */
class WinSvc
{
public:
  struct callbacks_t {
    WinSvcInitCB init;
    WinSvcRunCB  run;
    WinSvcStopCB stop;

    callbacks_t() : init(NULL), run(NULL), stop(NULL) {}
    callbacks_t(WinSvcInitCB initCB, WinSvcRunCB runCB, WinSvcStopCB stopCB)
      : init(initCB), run(runCB), stop(stopCB) {}
  };

// Utilities
public:
  static void install(const std::string& svcName) throw(WinSvcException);
  static void install(const std::string& svcName,
                      const std::string& binaryPath) throw(WinSvcException);
  static void remove(const std::string& svcName) throw(WinSvcException);
  static void enable(const std::string& svcName) throw(WinSvcException);
  static void disable(const std::string& svcName) throw(WinSvcException);

// Instance handling
public:
  static WinSvc* instance(const std::string& svcName,
                          const callbacks_t& callbacks) throw(WinSvcException);
  static WinSvc* instance() throw(WinSvcException);
private:
  static WinSvc* _instance;

// Instance methods
private:
  WinSvc(const std::string& svcName, const callbacks_t& callbacks);
public:
  void run() throw(WinSvcException);

// Windows service methods
private:
  static void WINAPI SvcCtrlHandlerCB(DWORD ctrl); 
  void SvcCtrlHandler(DWORD ctrl);
  static void WINAPI SvcMainCB(DWORD argc, LPTSTR* argv);
  void SvcMain(DWORD argc, LPTSTR* argv);

  void ReportSvcStatus(DWORD, DWORD, DWORD );
  void SvcInit(DWORD argc, LPTSTR* argv);
  void SvcReportEvent(LPTSTR );

private:
  const std::string       _svcName;
  const callbacks_t       _callbacks;
  SERVICE_STATUS          gSvcStatus; 
  SERVICE_STATUS_HANDLE   gSvcStatusHandle; 
  HANDLE                  ghSvcStopEvent;
};

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

/**
 * Install the current running binary as a Windows Service.
 *
 * @param svcName name of the service as shown to the user
 */
void WinSvc::install(const std::string& svcName) throw(WinSvcException)
{
  TCHAR szPath[MAX_PATH];
  
  if(!GetModuleFileName(NULL, szPath, MAX_PATH)) {
    throw WinSvcException("Cannot install service", GetLastError());
  }

  WinSvc::install(svcName, szPath);
}

/**
 * Install the binary at binaryPath as a Windows Service.
 *
 * @param svcName name of the service as shown to the user
 * @param binaryPath binary to install as a Windows Service
 */
void WinSvc::install(const std::string& svcName,
                     const std::string& binaryPath) throw(WinSvcException)
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager( 
    NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS    // full access rights
  );
 
  if (!schSCManager) {
    throw WinSvcException("OpenSCManager failed", GetLastError());
  }

  // Create the service
  schService = CreateService( 
    schSCManager,              // SCM database 
    svcName.c_str(),           // name of service 
    svcName.c_str(),           // service name to display 
    SERVICE_ALL_ACCESS,        // desired access 
    SERVICE_WIN32_OWN_PROCESS, // service type 
    SERVICE_DEMAND_START,      // start type 
    SERVICE_ERROR_NORMAL,      // error control type 
    binaryPath.c_str(),        // path to service's binary 
    NULL,                      // no load ordering group 
    NULL,                      // no tag identifier 
    NULL,                      // no dependencies 
    NULL,                      // LocalSystem account 
    NULL                       // no password 
  );
 
  if (!schService) {
    DWORD errorCode = GetLastError();
	CloseServiceHandle(schSCManager);
    throw WinSvcException("CreateService failed", errorCode); 
  }

  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);
}

/**
 * Delete a Windows Service.
 *
 * @param svcName name of the service as shown to the user
 */
void WinSvc::remove(const std::string& svcName) throw(WinSvcException)
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager( 
    NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS    // full access rights 
  );
 
    if (!schSCManager) {
      throw WinSvcException("OpenSCManager failed", GetLastError());
    }

    // Get a handle to the service.
    schService = OpenService( 
      schSCManager,       // SCM database 
      svcName.c_str(),    // name of service 
      DELETE              // need delete access 
    );

    if (!schService) { 
      DWORD errorCode = GetLastError();
      throw WinSvcException("OpenService failed", errorCode); 
    }

    // Delete the service.
    if (!DeleteService(schService)) {
      throw WinSvcException("DeleteService failed", GetLastError()); 
    }
 
    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

/**
 * Enable a Windows Service.
 *
 * @param svcName name of the service as shown to the user
 */
void WinSvc::enable(const std::string& svcName) throw(WinSvcException)
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager( 
    NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS    // full access rights 
  );
 
  if (!schSCManager) {
    throw WinSvcException("OpenSCManager failed", GetLastError());
  }

  // Get a handle to the service.
  schService = OpenService( 
    schSCManager,            // SCM database 
    svcName.c_str(),         // name of service 
    SERVICE_CHANGE_CONFIG    // need change config access
  );
 
  if (!schService) { 
    DWORD errorCode = GetLastError();
    throw WinSvcException("OpenService failed", errorCode); 
    CloseServiceHandle(schSCManager);
  }

  // Change the service start type.
  if (! ChangeServiceConfig( 
        schService,            // handle of service 
        SERVICE_NO_CHANGE,     // service type: no change 
        SERVICE_DEMAND_START,  // service start type 
        SERVICE_NO_CHANGE,     // error control: no change 
        NULL,                  // binary path: no change 
        NULL,                  // load order group: no change 
        NULL,                  // tag ID: no change 
        NULL,                  // dependencies: no change 
        NULL,                  // account name: no change 
        NULL,                  // password: no change 
        NULL)                  // display name: no change
    ) {
    throw WinSvcException("ChangeServiceConfig failed", GetLastError()); 
  }

  CloseServiceHandle(schService); 
  CloseServiceHandle(schSCManager);
}

/**
 * Disable a Windows Service.
 *
 * @param svcName name of the service as shown to the user
 */
void WinSvc::disable(const std::string& svcName) throw(WinSvcException)
{
  SC_HANDLE schSCManager;
  SC_HANDLE schService;

  // Get a handle to the SCM database. 
  schSCManager = OpenSCManager( 
    NULL,                    // local computer
    NULL,                    // ServicesActive database 
    SC_MANAGER_ALL_ACCESS    // full access rights 
  );
 
  if (!schSCManager) {
    throw WinSvcException("OpenSCManager failed", GetLastError());
  }

  // Get a handle to the service.
  schService = OpenService( 
    schSCManager,            // SCM database 
    svcName.c_str(),         // name of service 
    SERVICE_CHANGE_CONFIG    // need change config access
  );
 
  if (!schService) {
    DWORD errorCode = GetLastError();
    throw WinSvcException("OpenService failed", errorCode);
    CloseServiceHandle(schSCManager);
  }

  // Change the service start type.
  if (! ChangeServiceConfig(
        schService,            // handle of service
        SERVICE_NO_CHANGE,     // service type: no change
        SERVICE_DISABLED,      // service start type
        SERVICE_NO_CHANGE,     // error control: no change
        NULL,                  // binary path: no change
        NULL,                  // load order group: no change
        NULL,                  // tag ID: no change
        NULL,                  // dependencies: no change
        NULL,                  // account name: no change
        NULL,                  // password: no change
        NULL)                  // display name: no change
    ) {
    throw WinSvcException("ChangeServiceConfig failed", GetLastError());
  }

  CloseServiceHandle(schService);
  CloseServiceHandle(schSCManager);
}

//-----------------------------------------------------------------------------
// Instance handling
//-----------------------------------------------------------------------------

/**
 * Create the instance of WinSvc. Call exactly once.
 *
 * @param svcName name of the Windows service
 */
WinSvc* WinSvc::instance(const std::string& svcName,
                         const callbacks_t& callbacks) throw(WinSvcException)
{
  _instance = new WinSvc(svcName, callbacks);

  return _instance;
}

/**
 * Get the instance of WinSvc. Always call the creation instance() before
 * calling this method.
 */
WinSvc* WinSvc::instance() throw(WinSvcException)
{
  return _instance;
}

/** Singleton instance of WinSvc */
WinSvc* WinSvc::_instance = NULL;

//-----------------------------------------------------------------------------
// Instance methods
//-----------------------------------------------------------------------------

/**
 * Construct a WinSvc.
 *
 * @svcName name of the Windows service
 */
WinSvc::WinSvc(const std::string& svcName,
               const WinSvc::callbacks_t& callbacks) : _svcName(svcName),
			                                           _callbacks(callbacks),
                                                       ghSvcStopEvent(NULL)
{
  // Change directory to %PROGRAMDATA%\ammo-gateway
  const int MAX_ENV_VAR_SIZE = 32767;
  char* progdata = new char[MAX_ENV_VAR_SIZE];
  GetEnvironmentVariable("PROGRAMDATA", progdata, MAX_ENV_VAR_SIZE);
  std::string dir(progdata);
  dir += "\\ammo-gateway";
  if (!SetCurrentDirectory(dir.c_str())) {
    std::string errormsg;
    errormsg = "SetCurrentDirectory(" + dir + ")";
    SvcReportEvent((char*) errormsg.c_str());
  }
  delete[] progdata;
}

/**
 * Run the Windows service.
 */
void WinSvc::run() throw(WinSvcException)
{
  // Create the service table
  SERVICE_TABLE_ENTRY DispatchTable[] = 
  { 
    { (char*) this->_svcName.c_str(), (LPSERVICE_MAIN_FUNCTION) WinSvc::SvcMainCB },
    { NULL, NULL }
  };

  // Start the Windows service dispatcher.
  // This call returns when the service has stopped. 
  if (!StartServiceCtrlDispatcher( DispatchTable )) { 
    WinSvcException e("Failed to start service dispatcher", GetLastError());
    // TODO SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
    throw e;
  }
}

//-----------------------------------------------------------------------------
// Instance methods
//-----------------------------------------------------------------------------

/**
 * The service control handler callback is called whenever a control code is
 * send to the service using the ControlService function.
 *
 * @param ctrl control code
 */
void WINAPI WinSvc::SvcCtrlHandlerCB(DWORD ctrl)
{
  instance()->SvcCtrlHandler(ctrl);
}

/**
 * The service control handler is called by the service control handler calback
 * which is called by the ControlService function.
 *
 * @param ctrl control code
 */
void WinSvc::SvcCtrlHandler(DWORD ctrl)
{
  // Handle the requested control code.
  switch(ctrl) 
  {  
  case SERVICE_CONTROL_STOP: 
    ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

    // Signal the service to stop.
	this->_callbacks.stop();

    SetEvent(ghSvcStopEvent);
    ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
         
    return;
 
  case SERVICE_CONTROL_INTERROGATE: 
    break; 
 
  default: 
     break;
  } 
}

/**
 * Windows service entry point (callback for the dispatch table)
 *
 * @param argc argument count
 * @param argv arguments as a list of strings
 */
void WINAPI WinSvc::SvcMainCB(DWORD argc, LPTSTR* argv)
{
  instance()->SvcMain(argc, argv);
}

/**
 * Windows service entry point
 *
 * @param argc argument count
 * @param argv argments as a list of strings
 */
void WinSvc::SvcMain(DWORD argc, LPTSTR* argv)
{
  // Register the handler function for the service
  gSvcStatusHandle = RegisterServiceCtrlHandler(this->_svcName.c_str(), SvcCtrlHandlerCB);
  if(!gSvcStatusHandle) {
    SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
    return;
  }

  // These SERVICE_STATUS members remain as set here
  gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  gSvcStatus.dwServiceSpecificExitCode = 0;

  // Report initial status to the SCM
  ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

  // Perform service-specific initialization and work.
  SvcInit(argc, argv);
}

/**
 * Service code
 *
 * @param argc argument count
 * @param argv argments as a list of strings
 */
void WinSvc::SvcInit(DWORD argc, LPTSTR* argv)
{
  // TO_DO: Declare and set any required variables.
  //   Be sure to periodically call ReportSvcStatus() with 
  //   SERVICE_START_PENDING. If initialization fails, call
  //   ReportSvcStatus with SERVICE_STOPPED.

  // Create an event. The control handler function, SvcCtrlHandler,
  // signals this event when it receives the stop control code.
  ghSvcStopEvent = CreateEvent(
                               NULL,    // default security attributes
                               TRUE,    // manual reset event
                               FALSE,   // not signaled
                               NULL);   // no name

  if (!ghSvcStopEvent) {
    ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
    return;
  }

  // Perform initialization
  if (!this->_callbacks.init(argc, argv)) {
    ReportSvcStatus( SERVICE_STOPPED, ERROR_APP_INIT_FAILURE, 0 );
    return;
  }

  // Report running status when initialization is complete.
  ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

  // Perform work until service stops.
  this->_callbacks.run();

  while(1) {
    // Check whether to stop the service.
    WaitForSingleObject(ghSvcStopEvent, INFINITE);

    ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
    return;
  }
}

/**
 * Sets the current service status and reports it to the SCM.
 *
 * @param dwCurrentState The current state (see SERVICE_STATUS)
 * @param dwWin32ExitCode The system error code
 * @param dwWaitHint Estimated time for pending operation, in milliseconds
 */
void WinSvc::ReportSvcStatus(DWORD dwCurrentState,
                             DWORD dwWin32ExitCode,
                             DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

/** 
 * Logs messages to the event log
 *
 * @param szFunction name of function that failed
 */
void WinSvc::SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];
	const DWORD SVC_ERROR = 0xE000DEAD;  // E marks customer error, DEAD is ours to do with as we please

    hEventSource = RegisterEventSource(NULL, this->_svcName.c_str());

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = this->_svcName.c_str();
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        // event log handle
                    EVENTLOG_ERROR_TYPE, // event type
                    0,                   // event category
                    SVC_ERROR,           // event identifier
                    NULL,                // no security identifier
                    2,                   // size of lpszStrings array
                    0,                   // no binary data
                    lpszStrings,         // array of strings
                    NULL);               // no binary data

        DeregisterEventSource(hEventSource);
    }
}
