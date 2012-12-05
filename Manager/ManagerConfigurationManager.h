#ifndef MANAGERCONFIGURATIONMANAGER_H
#define MANAGERCONFIGURATIONMANAGER_H

#include <string>
#include <vector>
#include "ConfigurationManager.h"
#include "ServiceDesc.h"

class ManagerConfigurationManager : public ConfigurationManager
{
public:
    static ManagerConfigurationManager* getInstance();

    std::vector<ServiceDesc_t> getServices() { return services; }

protected:
    void init();
    void decode(const Json::Value& root);

private:
    ManagerConfigurationManager();

    std::vector<ServiceDesc_t> services;

    static ManagerConfigurationManager* sharedInstance;
};

#endif // MANAGERCONFIGURATIONMANAGER_H
