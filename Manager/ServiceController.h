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
