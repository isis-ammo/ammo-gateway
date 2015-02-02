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

#include "AndroidPluginConfigurationManager.h"
#include "json/value.h"
#include "log.h"

using namespace std;

const char *CONFIG_FILE = "AndroidPluginConfig.json";

AndroidPluginConfigurationManager *AndroidPluginConfigurationManager::sharedInstance = NULL;

AndroidPluginConfigurationManager::AndroidPluginConfigurationManager(const char* configFileName) : ConfigurationManager(configFileName) {
  init();
}

void AndroidPluginConfigurationManager::init()
{
  heartbeatTimeout = 100;
}

void AndroidPluginConfigurationManager::decode(const Json::Value& root)
{
  //TODO:  This really ought to be reading a uint, but LibJSON apparently isn't
  //       handling uints in any kind of a sane, predictable way.
  CM_DecodeInt( root, "HeartbeatTimeout", heartbeatTimeout);

  LOG_INFO("Android Plugin Configuration: ");
  LOG_INFO("  Heartbeat Timeout: " << heartbeatTimeout);
}

unsigned int AndroidPluginConfigurationManager::getHeartbeatTimeout() {
  return heartbeatTimeout;
}

AndroidPluginConfigurationManager* AndroidPluginConfigurationManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AndroidPluginConfigurationManager(CONFIG_FILE);
    sharedInstance->populate();
  }
  return sharedInstance;
}
AndroidPluginConfigurationManager* AndroidPluginConfigurationManager::getInstance(std::string configfile) {
  if(sharedInstance == NULL) {
    sharedInstance = new AndroidPluginConfigurationManager(configfile.c_str());
    sharedInstance->populate();
  }
  return sharedInstance;
}
