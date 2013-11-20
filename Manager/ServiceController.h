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

#ifndef SERVICECONTROLLER_H
#define SERVICECONTROLLER_H

#include <QString>

/** Controller class for starting and stopping system services. */
class ServiceController
{
public:
    /**
     * Start the service.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if successfully started, false otherwise
     */
    virtual bool start(QString& errMsg) = 0;

    /**
     * Stop the service.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if successfully stopped, false otherwise
     */
    virtual bool stop(QString& errMsg) = 0;

    /**
     * Inquire if the service is running.
     *
     * @param errMsg (out) error message in the case of failure
     * @return true if running, false otherwise
     */
    virtual bool running(QString& errMsg) = 0;

    /**
     * Get the human name of the service.
     *
     * @return human service name
     */
    QString humanName() const { return _humanName; }

    /**
     * Get the system name of the service.
     *
     * @return system service name
     */
    QString systemName() const { return _systemName; }

    /**
     * Create a service controller.
     *
     * @param humanName human service name
     * @param systemName system service name
     */
    static ServiceController* create(const QString& humanName, const QString& systemName);

protected:
    ServiceController(const QString& humanName, const QString& systemName) : _humanName(humanName), _systemName(systemName) {}

private:
    const QString _humanName;   /// name the service goes by to a human
    const QString _systemName;  /// name the service goes by to the system
};

#endif // SERVICECONTROLLER_H
