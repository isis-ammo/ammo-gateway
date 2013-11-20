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

using namespace std;

const char *CONFIG_FILE = "GatewayConfig.json";

ammo::gateway::internal::GatewayConfigurationManager *ammo::gateway::internal::GatewayConfigurationManager::sharedInstance = NULL;

ammo::gateway::internal::GatewayConfigurationManager::GatewayConfigurationManager(const char* configFileName) : ConfigurationManager(configFileName) {
  init();
}

void ammo::gateway::internal::GatewayConfigurationManager::init()
{
  gatewayAddress = "127.0.0.1";
  gatewayInterface = "0.0.0.0";
  gatewayPort = 12475;
}

void ammo::gateway::internal::GatewayConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString( root, "GatewayInterface", gatewayInterface );
  CM_DecodeString( root, "GatewayAddress",   gatewayAddress   );
  CM_DecodeInt   ( root, "GatewayPort",      gatewayPort      );

  LOG_INFO("Gateway Configuration: ");
  LOG_INFO("  Interface: " << gatewayInterface);
  LOG_INFO("  Address: " << gatewayAddress);
  LOG_INFO("  Port: " << gatewayPort);
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayAddress() {
  return gatewayAddress;
}

std::string ammo::gateway::internal::GatewayConfigurationManager::getGatewayInterface() {
  return gatewayInterface;
}

int ammo::gateway::internal::GatewayConfigurationManager::getGatewayPort() {
  return gatewayPort;
}

ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(CONFIG_FILE);
	sharedInstance->populate();
  }
  return sharedInstance;
}
ammo::gateway::internal::GatewayConfigurationManager* ammo::gateway::internal::GatewayConfigurationManager::getInstance(std::string configfile) {
  if(sharedInstance == NULL) {
    sharedInstance = new GatewayConfigurationManager(configfile.c_str());
	sharedInstance->populate();
  }
  return sharedInstance;
}
