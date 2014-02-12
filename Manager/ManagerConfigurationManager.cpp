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
