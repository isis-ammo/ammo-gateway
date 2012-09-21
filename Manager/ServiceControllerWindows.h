#ifndef SERVICECONTROLLERWINDOWS_H
#define SERVICECONTROLLERWINDOWS_H

#include <Windows.h>
#include <QHash>
#include "ServiceController.h"

/**
 * Controller class for starting and stopping services on Microsoft Windows.
 * Do not use directly. Use ServiceController instead.
 */
class ServiceControllerWindows : public ServiceController
{
public:
    ServiceControllerWindows(const QString& humanName, const QString& systemName);
    ~ServiceControllerWindows();

    bool start(QString& errMsg);
    bool stop(QString& errMsg);
    bool running(QString& errMsg);

private:
    static SC_HANDLE _svcMan;  /// handle to Windows Service Manager
    SC_HANDLE _svc;     /// handle to a Windows Service

    /**
     * Open the Windows Service Manager and store a handle to it in _svcMan.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if open, false otherwise
     */
    static bool openSvcMan(QString& errMsg);

    /**
     * Open the Windows Service described as systemName() and store a handle to it in _svc.
     * You must call openSvcMan() first.
     *
     * @param errMsg (out) error message in case of failure
     * @return true if open, false otherwise
     */
    bool openSvc(QString& errMsg);

    /**
     * Get the running state of the Windows Service described as systemName().
     * You must call openSvc() first.
     *
     * @param state (out) running state per SERVICE_STATUS_PROCESS.dwCurrentState
     * @param errMsg (out) error message in the case of failure
     * @return true on success, false otherwise
     */
    bool getSvcState(DWORD& state, QString& errMsg);

    /**
     * Augmentation of Windows' GetLastError() using QString instead of int.
     *
     * @return human readable error message
     */
    static QString getLastErrorString();

    static QString LPTSTR2QString(LPTSTR s);
};

#endif // SERVICECONTROLLERWINDOWS_H
