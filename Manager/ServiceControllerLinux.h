#ifndef SERVICECONTROLLERLINUX_H
#define SERVICECONTROLLERLINUX_H

#include "ServiceController.h"

/**
 * Controller class for starting and stopping services on Linux.
 * Do not use directly. Use ServiceController instead.
 */
class ServiceControllerLinux : public ServiceController
{
public:
    ServiceControllerLinux(const QString& humanName, const QString& systemName);

    bool start(QString& errMsg);
    bool stop(QString& errMsg);
    bool running(QString& errMsg);
};

#endif // SERVICECONTROLLERLINUX_H
