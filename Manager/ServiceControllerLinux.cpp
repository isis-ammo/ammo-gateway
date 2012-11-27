#include "ServiceControllerLinux.h"

ServiceControllerLinux::ServiceControllerLinux(const QString& humanName, const QString& systemName) :
    ServiceController(humanName, systemName)
{
}

bool ServiceControllerLinux::start(QString& errMsg)
{
    errMsg = "Not implemented";
    return false;
}

bool ServiceControllerLinux::stop(QString& errMsg)
{
    errMsg = "Not implemented";
    return false;
}

bool ServiceControllerLinux::running(QString& errMsg)
{
    errMsg = "Not implemented";
    return false;
}
