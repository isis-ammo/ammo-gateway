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

#include "GatewayConfigurationManager.h"
#include "json/value.h"
#include "log.h"

static const char *CONFIG_FILE = "GatewayConfig.json";

using namespace std;

GatewayConfigurationManager *GatewayConfigurationManager::sharedInstance = NULL;

GatewayConfigurationManager::GatewayConfigurationManager() : ConfigurationManager(CONFIG_FILE) {
  init();
}

void GatewayConfigurationManager::init()
{
  gatewayAddress = "127.0.0.1";
  gatewayInterface = "0.0.0.0";
  gatewayPort = 12475;
  crossGatewayId = "DefaultGateway";
  crossGatewayServerInterface = "127.0.0.1";
  crossGatewayServerPort = 47543;
  crossGatewayParentAddress = "";
  crossGatewayParentPort = 47543;
}

void GatewayConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "GatewayInterface",             gatewayInterface            );
  CM_DecodeString ( root, "GatewayAddress",               gatewayAddress              );
  CM_DecodeInt    ( root, "GatewayPort",                  gatewayPort                 );
  CM_DecodeString ( root, "CrossGatewayId",               crossGatewayId              );
  CM_DecodeString ( root, "CrossGatewayServerInterface",  crossGatewayServerInterface );
  CM_DecodeInt    ( root, "CrossGatewayServerPort",       crossGatewayServerPort      );
  CM_DecodeString ( root, "CrossGatewayParentAddress",    crossGatewayParentAddress   );
  CM_DecodeInt    ( root, "CrossGatewayParentPort",       crossGatewayParentPort      );

  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
  LOG_INFO("  CrossGatewayId: " << crossGatewayId);
  LOG_INFO("  CrossGatewayServerInterface: " << crossGatewayServerInterface);
  LOG_INFO("  CrossGatewayServerPort: " << crossGatewayServerPort);
  LOG_INFO("  CrossGatewayParentAddress: " << crossGatewayParentAddress);
  LOG_INFO("  CrossGatewayParentPort: " << crossGatewayParentPort);
}

std::string GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

std::string GatewayConfigurationManager::getCrossGatewayId() {
  return crossGatewayId;
}

int GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

std::string GatewayConfigurationManager::getCrossGatewayServerInterface() {
  return crossGatewayServerInterface;
}

int GatewayConfigurationManager::getCrossGatewayServerPort() {
  return crossGatewayServerPort;
}

std::string GatewayConfigurationManager::getCrossGatewayParentAddress() {
  return crossGatewayParentAddress;
}

int GatewayConfigurationManager::getCrossGatewayParentPort() {
  return crossGatewayParentPort;
}

GatewayConfigurationManager* GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager();
	sharedInstance->populate();
  }
  return sharedInstance;
}
