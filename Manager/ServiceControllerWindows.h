/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
