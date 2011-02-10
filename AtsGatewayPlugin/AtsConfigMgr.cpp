

#include <iostream>
#include <sstream>
#include <fstream>

#include "log.h"
#include "AtsConfigMgr.h"

const char *ATS_CONFIG_FILE = "AtsPluginConfig.json";

AtsConfigMgr *AtsConfigMgr::sharedInstance = NULL;

AtsConfigMgr::AtsConfigMgr() : parsingSuccessful(false) 
{
  //LOG_TRACE("Parsing config file...");
  std::ifstream configFile(ATS_CONFIG_FILE);
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
  
  // LOG_INFO(" Connector Configuration: ");
  // LOG_INFO("  Host:      " << root->getAtsHost());
  // LOG_INFO("  Port:      " << root->getAtsPort());
  // LOG_INFO("  Base Dir:  " << root->getAtsBaseDir());
  // LOG_INFO("  Address:   " << root->getAtsUsername());
  // LOG_INFO("  Password:  " << root->getAtsPassword());
}

AtsConfigMgr* AtsConfigMgr::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AtsConfigMgr();
  }
  return sharedInstance;
}

std::string AtsConfigMgr::getGatewayConfig() const {
    if(! parsingSuccessful) return "";
    if(! root["GatewayConfig"].isString()) {
        LOG_ERROR("Error: GatewayConfig is missing or wrong type (should be string)");
        return "";
    }
    return root["GatewayConfig"].asString();
}

std::string AtsConfigMgr::getHost() const {
    if(! parsingSuccessful) return "flagon.aterrasys.com";
    if(! root["AtsHost"].isString()) {
        LOG_ERROR("Error: AtsHost is missing or wrong type (should be string)");
        return "localhost";
    }
    return root["AtsHost"].asString();
}


int AtsConfigMgr::getPort() const {
    if(! parsingSuccessful) return 80;
    if(! root["AtsPort"].isInt()) {
      LOG_ERROR("Error: AtsPort is missing or wrong type (should be integer)");
      return 80;
    } 
    return root["AtsPort"].asInt();
}


std::string AtsConfigMgr::getBaseDir() const {
    if(! parsingSuccessful) return "nevada/api/";
    if(! root["AtsBaseDir"].isString()) {
      LOG_ERROR("Error: AtsBaseDir is missing or wrong type (should be string)");
      return "nevada/api/";
    } 
    return root["AtsBaseDir"].asString();
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
    if(! root["AtsUserName"].isString()) {
      LOG_ERROR("Error: AtsUserName is missing or wrong type (should be string)");
      return "guest";
    } 
    return root["AtsUserName"].asString();
}

std::string AtsConfigMgr::getPassword() const {
    if(! parsingSuccessful) return "secret";
    if(! root["AtsPassword"].isString()) {
      LOG_ERROR("Error: AtsPassword is missing or wrong type (should be string)");
      return "secret";
    } 
    return root["AtsPassword"].asString();
}

std::pair<std::string, std::string> AtsConfigMgr::getCredentialsForUser(std::string username) {
  std::pair<std::string, std::string> newCredentials;
  //default username in case of error or missing credentials
  newCredentials.first = getUsername();
  newCredentials.second = getPassword();
  
  if( getUsername() != "") {
    if (root["UsernameMap"].isObject() 
     && root["UsernameMap"][username].isObject() 
     && root["UsernameMap"][username]["Username"].isString() 
     && root["UsernameMap"][username]["Password"].isString()) 
    {
      newCredentials.first = root["UsernameMap"][username]["Username"].asString();
      newCredentials.second = root["UsernameMap"][username]["Password"].asString();
    }
  }
  return newCredentials;
}

