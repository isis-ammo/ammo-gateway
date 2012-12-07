#include <string.h>
#include <tchar.h>
#include "ServiceControllerWindows.h"

SC_HANDLE ServiceControllerWindows::_svcMan = NULL;

ServiceControllerWindows::ServiceControllerWindows(const QString& humanName, const QString& systemName) :
    ServiceController(humanName, systemName),
    _svc(NULL)
{
}

ServiceControllerWindows::~ServiceControllerWindows()
{
    if (this->_svc) {
        CloseServiceHandle(this->_svc);
    }
    this->_svc = NULL;
}

bool ServiceControllerWindows::start(QString& errMsg)
{
    DWORD state;

    if (!openSvcMan(errMsg)) return false;
    if (!openSvc(errMsg)) return false;
    if (!getSvcState(state, errMsg)) return false;

    // If the service isn't stopped, this is pointless.
    if (state != SERVICE_STOPPED && state != SERVICE_STOP_PENDING) return true;  // already started or starting

    // Finish a pending stop
    while (state == SERVICE_STOP_PENDING) {
        Sleep(250);  // sleep 1/4 second
        if (!getSvcState(state, errMsg)) {
            errMsg = QString("Error watching state when awaiting a pending stop - ") + errMsg;
            return false;
        }
    }

    // Kickstart service
    if (!StartService(
           this->_svc,  // handle to service
           0,           // number of arguments
           NULL)) {     // arguments
        errMsg = QString("Can not start service - ") + getLastErrorString();
        return false;
    }

    // Wait for service to start (or fail to start)
    do {
        Sleep(250);  // sleep for 1/4 second
        if (!getSvcState(state, errMsg)) {
            errMsg = QString("Error watching state when starting service - ") + errMsg;
            return false;
        }
    } while (state == SERVICE_START_PENDING);

    // Check if service failed to start
    if (state != SERVICE_RUNNING) {
        errMsg = "Service failed to start";
        return false;
    }

    return true;
}

bool ServiceControllerWindows::stop(QString& errMsg)
{
	DWORD state;
	SERVICE_STATUS dummy;

    if (!openSvcMan(errMsg)) return false;
    if (!openSvc(errMsg)) return false;
	if (!getSvcState(state, errMsg)) return false;

	// Finish a pending stop
	while (state == SERVICE_STOP_PENDING) {
        Sleep(250);  // sleep 1/4 second
        if (!getSvcState(state, errMsg)) {
            errMsg = QString("Error watching state when awaiting a pending stop - ") + errMsg;
            return false;
        }
    }

	// If the service is already stopped, this is pointless.
	if (state == SERVICE_STOPPED) return true;  // already stopped

	// Signal the service to stop
	if (!ControlService(
           this->_svc,            // handle to service
           SERVICE_CONTROL_STOP,  // stop request
           &dummy)) {             // state (unused)
        errMsg = QString("Can not stop service - ") + getLastErrorString();
        return false;
    }

	// Wait for service to start (or fail to start)
    do {
        Sleep(250);  // sleep for 1/4 second
        if (!getSvcState(state, errMsg)) {
            errMsg = QString("Error watching state when stopping service - ") + errMsg;
            return false;
        }
	} while (state != SERVICE_STOPPED);

    return true;
}

bool ServiceControllerWindows::running(QString& errMsg)
{
	DWORD state;

    if (!openSvcMan(errMsg)) return false;
    if (!openSvc(errMsg)) return false;

	if (!getSvcState(state, errMsg)) return false;

	return state == SERVICE_RUNNING;
}

bool ServiceControllerWindows::openSvcMan(QString& errMsg)
{
    if (_svcMan) return true;

    _svcMan = OpenSCManager(
      NULL,                  // local computer
      NULL,                  // ServicesActive database
      SC_MANAGER_ALL_ACCESS  // full access rights
    );

    if (!_svcMan) {
        errMsg = QString("Can not open services database - ") + getLastErrorString();
        return false;
    }

    return true;
}

bool ServiceControllerWindows::openSvc(QString& errMsg)
{
    if (this->_svc) return true;

	this->_svc = OpenServiceA(
      _svcMan,                                   // SCM database
	  this->systemName().toStdString().c_str(),  // name of service
      SERVICE_ALL_ACCESS                         // full access rights
    );

    if (!this->_svc) {
        errMsg = QString("Can not open service - ") + getLastErrorString();
        return false;
    }

    return true;
}

bool ServiceControllerWindows::getSvcState(DWORD& state, QString& errMsg)
{
    SERVICE_STATUS_PROCESS status;
    DWORD dummy;

    if (!QueryServiceStatusEx(
           this->_svc,              // handle to service
           SC_STATUS_PROCESS_INFO,  // information level
           (LPBYTE) &status,        // address of status structure
           sizeof(status),          // size of status struction
           &dummy)) {               // size needed if buffer is too small (unused)
        errMsg = QString("Can not query current service status - ") + getLastErrorString();
        return false;
    }

    state = status.dwCurrentState;

    return true;
}

QString ServiceControllerWindows::getLastErrorString()
{
    QString msg;
    const DWORD errorCode = GetLastError();
    LPTSTR winMsg = NULL;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  errorCode,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &winMsg,
                  0,
                  NULL);

    if (winMsg) {
        QString formattedMsg = LPTSTR2QString(winMsg);
        msg = QString("Error %1: %2").arg(errorCode).arg(formattedMsg);
    }
    else {
        msg = QString("Error %1: Failed to get error message");
    }

    LocalFree(winMsg);

    return msg;
}

QString ServiceControllerWindows::LPTSTR2QString(LPTSTR s)
{
#ifdef UNICODE
    return QString::fromUtf16((const ushort*) s);
#else
    return QString::fromAscii(static_cast<const char *>(s));
#endif
}
