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
