#include <fstream>

#include <ace/OS_NS_sys_stat.h>
#include <ace/UUID.h>
#include <ace/SString.h>

#include "json/reader.h"
#include "json/value.h"
#include "log.h"

#include "PassConfigurationManager.h"
#include "PassGatewayReceiver.h"

const char *CONFIG_DIRECTORY = "ammo-gateway";
const char *PASS_CONFIG_FILE = "PassPluginConfig.json";

using namespace std;
using namespace ammo::gateway;

PassConfigurationManager *PassConfigurationManager::sharedInstance = 0;

PassConfigurationManager*
PassConfigurationManager::getInstance (PassGatewayReceiver *receiver,
                                       GatewayConnector *connector)
{
  if (sharedInstance == 0)
    {
      if (connector == 0 || receiver == 0)
        {
          LOG_ERROR ("First call to getInstance() must initialize"
                     " receiver and connector");
                     
          return 0;
        }
        
      sharedInstance =
        new PassConfigurationManager (receiver, connector);
    }
  
  return sharedInstance;
}

const std::string &
PassConfigurationManager::getPassServerAddress (void) const
{
  return passServerAddress;  
}

const std::string &
PassConfigurationManager::getPassTopic (void) const
{
  return passTopic;
}

const std::string &
PassConfigurationManager::getPassSubscriberId (void) const
{
  return passSubscriberId;
}

const std::string &
PassConfigurationManager::getPassPublisherId (void) const
{
  return passPublisherId;
}

const std::string &
PassConfigurationManager::getPassSubscriberInterface (void) const
{
  return passSubscriberInterface;
}

int
PassConfigurationManager::getPassSubscriberPort (void) const
{
  return passSubscriberPort;
}

const std::string &
PassConfigurationManager::getPassSubscriberAddress (void) const
{
  return passSubscriberAddress;
}

const std::string &
PassConfigurationManager::getPassPluginId (void) const
{
  return passPluginId;
}

PassGatewayReceiver *
PassConfigurationManager::getReceiver (void) const
{
  return receiver_;
}

PassConfigurationManager::PassConfigurationManager (PassGatewayReceiver *receiver,
                                                    GatewayConnector *connector)
  : receiver_ (receiver),
    connector_ (connector)
{
  LOG_TRACE ("Generating plugin UUID...");
  ACE_Utils::UUID *uuid =
    ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID ();
    
  this->passPluginId = uuid->to_string ()->c_str ();
  
  delete uuid;
  uuid = 0;
  
  LOG_TRACE ("Parsing config file...");
	
  string configFilename = findConfigFile ();
  
  if (configFilename != "")
    {
      ifstream configFile (configFilename.c_str ());
	
      if (configFile)
        {
          Json::Value root;
          Json::Reader reader;
          
          bool parsingSuccessful = reader.parse (configFile, root);
          
          if (parsingSuccessful)
            {
              if (root["PassServerAddress"].isString ())
                {
                  passServerAddress = root["PassServerAddress"].asString ();
                }
              else
                {
                  LOG_ERROR ("PassServerAddress is missing "
                             << "or wrong type (should be string)");
                }
              
              if (root["PassTopic"].isString ())
                {
                  passTopic = root["PassTopic"].asString ();
                }
              else
                {
                  LOG_ERROR ("PassTopic is missing or "
                             << "wrong type (should be string)");
                }
              
              if(root["PassSubscriberId"].isString ())
                {
                  passSubscriberId = root["PassSubscriberId"].asString ();
                }
              else
                {
                  LOG_ERROR ("PassSubscriberId is missing or "
                             << "wrong type (should be string)");
                }
              
              if(root["PassPublisherId"].isString ())
                {
                  passPublisherId = root["PassPublisherId"].asString ();
                }
              else
                {
                  LOG_ERROR ("PassPublisherId is missing or "
                             << "wrong type (should be string)");
                }
              
              if (root["PassSubscriberInterface"].isString ())
                {
                  passSubscriberInterface = root["PassSubscriberInterface"].asString ();
                }
              else
                {
                  LOG_ERROR ("PassSubscriberInterface is missing or "
                             << "wrong type (should be string)");
                }
              
              if (root["PassSubscriberPort"].isInt ())
                {
                  passSubscriberPort = root["PassSubscriberPort"].asInt ();
                }
              else
                {
                  LOG_ERROR ("PassSubscriberPort is missing or "
                             << "wrong type (should be integer)");
                }
              
              if (root["PassSubscriberAddress"].isString ())
                {
                  passSubscriberAddress = root["PassSubscriberAddress"].asString();
                }
              else
                {
                  LOG_ERROR ("PassSubscriberAddress is missing or "
                             << "wrong type (should be string)");
                }
                
              if (root["ContentTopic"].isString ())
                {
                  string content_type = root["ContentTopic"].asString ();
                  LOG_DEBUG ("Registering interest in " << content_type);
                  connector_->registerDataInterest (content_type, receiver_);
                }
              else
                {
                  LOG_ERROR ("ContentTopic is missing or "
                             << "wrong type (should be string)");
                }

              if (root["PassPluginID"].isString ())
                {
                  this->passPluginId = root["PassPluginID"].asString ();
                  LOG_DEBUG ("PASS Plugin ID set to " << this->passPluginId);
                }
              else
                {
                  LOG_ERROR ("PASS Plugin ID defaults to UUID " << this->passPluginId);
                }
            }
          else
            {
              LOG_ERROR ("JSON parsing error in config file '"
                         << PASS_CONFIG_FILE
                         << "'.  Using defaults.");
            }
            
          configFile.close ();
        }
      else
        {
          LOG_WARN ("Could not read from config file '"
                    << PASS_CONFIG_FILE
                    << "'.  Using defaults.");
        }
	  }
  else
    {
      LOG_WARN ("Using default configuration.");
    }
  
  LOG_DEBUG ("");
  LOG_DEBUG ("Pass Plugin Configuration: ");
  LOG_DEBUG ("  Pass Server Address: " << passServerAddress);
  LOG_DEBUG ("  Pass Topic: " << passTopic);
  LOG_DEBUG ("  Pass Subscriber ID: " << passSubscriberId);
  LOG_DEBUG ("  Pass Subscriber Interface: " << passSubscriberInterface);
  LOG_DEBUG ("  Pass Subscriber Port: " << passSubscriberPort);
  LOG_DEBUG ("  Pass Subscriber Address: " << passSubscriberAddress);
  LOG_DEBUG ("");
  
  receiver_->set_cfg_mgr (this);
}

string
PassConfigurationManager::findConfigFile (void)
{
  string filePath;
  ACE_stat statStruct;
  
  string home, gatewayRoot;
  
  char *homeC = ACE_OS::getenv ("HOME");
  
  if (homeC == 0)
    {
      home = "";
    }
  else
    {
      home = homeC;
    }
  
  char *gatewayRootC = ACE_OS::getenv ("GATEWAY_ROOT");
  
  if (gatewayRootC == 0)
    {
      gatewayRoot = "";
    }
  else
    {
      gatewayRoot = gatewayRootC;
    }
  
  filePath = PASS_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CONFIG_DIRECTORY + "/" + PASS_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = string("/etc/") + CONFIG_DIRECTORY + "/" + PASS_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + PASS_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + PASS_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = string("../etc/") + PASS_CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No config file found.");
              return "";
            }
          }
        }
      }
    }
  }
  
  LOG_INFO ("Using config file: " << filePath);
  return filePath;
}


