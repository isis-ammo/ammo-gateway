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
