#include "ManagerConfigurationManager.h"

static const char* CONFIG_FILE = "ManagerConfig.json";

ManagerConfigurationManager* ManagerConfigurationManager::sharedInstance = NULL;

ManagerConfigurationManager* ManagerConfigurationManager::getInstance()
{
    if (!sharedInstance) {
        sharedInstance = new ManagerConfigurationManager();
        sharedInstance->populate();
    }

    return sharedInstance;
}

ManagerConfigurationManager::ManagerConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
    init();
}

void ManagerConfigurationManager::init()
{
    services.clear();
}

void ManagerConfigurationManager::decode(const Json::Value& root)
{
    Json::Value jsonServices;

    CM_DecodeArray(root, "Services", jsonServices);

    for (int i = 0; i < jsonServices.size(); i++) {
        Json::Value dummy;
        Json::Value jsonService = jsonServices.get(Json::Value::UInt(i), dummy);
        ServiceDesc_t svcDesc;

        CM_DecodeString(jsonService, "HumanName", svcDesc.humanName);
        CM_DecodeString(jsonService, "SvcName",   svcDesc.svcName);
        CM_DecodeString(jsonService, "ConfigName", svcDesc.configName);
        CM_DecodeString(jsonService, "LogName", svcDesc.logName);

        services.push_back(svcDesc);
    }

    // TODO: Log configuration
}
