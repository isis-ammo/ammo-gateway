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
