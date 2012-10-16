#include "ServiceController.h"

#ifdef WINDOWS
  #include "ServiceControllerWindows.h"
#elif LINUX
  #include "ServiceControllerLinux.h"
#endif

ServiceController* ServiceController::create(const QString& humanName, const QString& systemName)
{
#ifdef WINDOWS
    return new ServiceControllerWindows(humanName, systemName);
#elif LINUX
    return new ServiceControllerLinux(humanName, systemName);
#else
    #error "Not implemented"
#endif
}
