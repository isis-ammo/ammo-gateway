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



#include <iostream>
#include <sstream>
#include <fstream>

#include "log.h"
#include "AtsConfigMgr.h"

#include <ace/OS_NS_sys_stat.h> 
#include <ace/OS_NS_stdlib.h>

using namespace std;

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *ATS_CONFIG_FILE = "AtsPluginConfig.json";

AtsConfigMgr *AtsConfigMgr::sharedInstance = NULL;

AtsConfigMgr::AtsConfigMgr() : parsingSuccessful(false) 
{
  //LOG_TRACE("Parsing config file...");
  string filename = findConfigFile();
  if(filename != "") {
    std::ifstream configFile(filename.c_str());
    if(! configFile) {
         LOG_WARN("Could not read from config file '" << ATS_CONFIG_FILE << "'.  Using defaults.");
         return;
    } 
    Json::Reader reader;
      
    parsingSuccessful = reader.parse(configFile, root);
      
    if(! parsingSuccessful) {
       LOG_ERROR("JSON parsing error in config file '" << ATS_CONFIG_FILE << "'.  Using defaults.");
       return;
    }
    configFile.close();
  } else {
    LOG_WARN("Using default configuration.");
  }
}

AtsConfigMgr* AtsConfigMgr::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AtsConfigMgr();
  }
  return sharedInstance;
}

bool AtsConfigMgr::hasGatewayConfig() const {
    if(! parsingSuccessful) return false;
    return root["GatewayConfig"].isString();
}

std::string AtsConfigMgr::getGatewayConfig() const {
    if(! parsingSuccessful) return "";
    if(! root["GatewayConfig"].isString()) {
        return "";
    }
    LOG_WARN("Warn: GatewayConfig is being overridden " + root["GatewayConfig"].asString());
    return root["GatewayConfig"].asString();
}

std::string AtsConfigMgr::getHost() const {
    if(! parsingSuccessful) return "flagon.aterrasys.com";
    if(! root["Host"].isString()) {
        LOG_WARN("Error: Host is missing or wrong type (should be string)");
        return "localhost";
    }
    return root["Host"].asString();
}


int AtsConfigMgr::getPort() const {
    if(! parsingSuccessful) return 80;
    if(! root["Port"].isInt()) {
      LOG_WARN("Error: Port is missing or wrong type (should be integer)");
      return 80;
    } 
    return root["Port"].asInt();
}


std::string AtsConfigMgr::getBaseDir() const {
    if(! parsingSuccessful) return "nevada/api/";
    if(! root["BaseDir"].isString()) {
      LOG_WARN("Error: BaseDir is missing or wrong type (should be string)");
      return "nevada/api/";
    } 
    return root["BaseDir"].asString();
}

std::string AtsConfigMgr::getUrl() const {
  std::ostringstream url;  
  url << "http://" << getHost() << ":" << getPort() << "/" << getBaseDir();
  return url.str();
}

std::string AtsConfigMgr::getUrl(std::string suffix) const {
  return getUrl() + "/" + suffix;
}

std::string AtsConfigMgr::getUsername() const {
    if(! parsingSuccessful) return "guest";
    if(! root["UserName"].isString()) {
      LOG_WARN("Error: UserName is missing or wrong type (should be string)");
      return "guest";
    } 
    return root["UserName"].asString();
}

std::string AtsConfigMgr::getPassword() const {
    if(! parsingSuccessful) return "secret";
    if(! root["Password"].isString()) {
      LOG_WARN("Error: Password is missing or wrong type (should be string)");
      return "secret";
    } 
    return root["Password"].asString();
}

std::string AtsConfigMgr::getUsername(std::string alias) const {
    if(! parsingSuccessful) return "guest";
    std::pair<std::string, std::string> credentials = getUserCredentials(alias);
    return credentials.first;
}

std::string AtsConfigMgr::getPassword(std::string alias) const {
    if(! parsingSuccessful) return "secret";
    std::pair<std::string, std::string> credentials = getUserCredentials(alias);
    return credentials.second;
}

std::string AtsConfigMgr::getHttpAuth() const {
    if(! parsingSuccessful) return ":";
    return getUsername()+":"+getPassword();
}

std::string AtsConfigMgr::getHttpAuth(std::string alias) const {
    if(! parsingSuccessful) return ":";
    std::pair<std::string, std::string> credentials = getUserCredentials(alias);
    return credentials.first+":"+credentials.second;
}


std::pair<std::string, std::string> AtsConfigMgr::getUserCredentials(std::string alias) const {
  std::pair<std::string, std::string> credentials;
  //default alias in case of error or missing credentials
  credentials.first = getUsername();
  credentials.second = getPassword();
  
  if( credentials.first == "") return credentials;

  if (! root["UsernameMap"].isObject()) return credentials; 
  if (! root["UsernameMap"][alias].isObject()) return credentials; 
  if (! root["UsernameMap"][alias]["Username"].isString()) return credentials; 
      credentials.first = root["UsernameMap"][alias]["Username"].asString();
  if (! root["UsernameMap"][alias]["Password"].isString()) return credentials; 
      credentials.second = root["UsernameMap"][alias]["Password"].asString();
  return credentials;
}

std::string AtsConfigMgr::findConfigFile() {
  std::string filePath;
  ACE_stat statStruct;
  
  string home, gatewayRoot;
  
  char *homeC = ACE_OS::getenv("HOME");
  if(homeC == NULL) {
    home = "";
  } else {
    home = homeC;
  }
  
  char *gatewayRootC = ACE_OS::getenv("GATEWAY_ROOT");
  if(gatewayRootC == NULL) {
    gatewayRoot = "";
  } else {
    gatewayRoot = gatewayRootC;
  }
  
  filePath = ATS_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + ATS_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + ATS_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + ATS_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + ATS_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + ATS_CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No config file found.");
              return "";
            }
          }
        }
      }
    }
  }
  
  LOG_INFO("Using config file: " << filePath);
  return filePath;
}
