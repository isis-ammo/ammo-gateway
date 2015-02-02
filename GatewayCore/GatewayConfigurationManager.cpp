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
