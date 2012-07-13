#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H

#include <string>

#include "json/value.h"


/**
 * The ConfigurationManager is the base class for configuration management
 * for AMMO Gateway and its plugins. Configuration files are stored as JSON
 * files in one of the following places (searched in order).
 *
 * Windows:
 *   1) The current working directory
 *   2) The user's configuration directory (Roaming appdata directory/ammo-gateway)
 *   3) The all users configuration directory (i.e. C:\ProgramData\ammo-gateway on Vista/7)
 *   Fallback locations (don't rely on these; they may change or disappear in a
 *   future release.  Gateway installation should put the config file into
 *   a location that's searched by default):
 *   4) $GATEWAY_ROOT/etc
 *   5) $GATEWAY_ROOT/build/etc
 *   6) ../etc
 * Linux:
 *   1) The current working directory
 *   2) ~/.ammo-gateway/
 *   3) /etc/ammo-gateway/
 *   Fallback locations (don't rely on these; they may change or disappear in
 *   a future release.  Gateway installation should put the config file into
 *   a location that's searched by default):
 *   4) $GATEWAY_ROOT/etc
 *   5) $GATEWAY_ROOT/build/etc
 *   6) ../etc
 *
 * This class handles finding and opening the configuration file. To use this
 * class, do the following:
 *   1) Subclass this class - name it [MyPlugin]ConfigurationManager
 *   2) Provide the short file name - typically [MyPlugin]Config.json
 *   3) Provide default values for your options in init()
 *   4) Decode config values in decode()
 *   5) LOG_INFO() your configuration
 *   6) Provide getters for your config values
 *   7) Instantiate your subclass and call populate - typically as a singleton
 *
 * Your decode() method should make use of the CM_Decode*() macros below. The
 * first parameter is the Json::Value holding the option. The second parameter
 * is the char* name of the option. The third parameter is the destination
 * variable to hold the option value. This variable's type is dependent on
 * which CM_Decode*() call is made.
 */
class ConfigurationManager {
public:
  /**
   * Search the configuration file locations for the specified config file.
   *
   * @param configfile configuration file on which to search
   * @return the full path to the config file or an empty string
   */
  static std::string findConfigFile(const std::string& configfile);

  /**
   * Populate the configuration manager. This must be called before reading
   * any option from the configuration manager to ensure valid values.
   *
   * Options are first initilized to their default values. Then the
   * configuration file is found, opened, and its values decoded and stored.
   */
  void populate();

  /**
   * Search the configuration file locations for the config file.
   *
   * @return the full path to the config file or an empty string
   */
  std::string findConfigFile();
  
protected:
  /**
   * ConfigurationManager constructor.
   *
   * Pass in the short name of the config file that populates this manager.
   * The full path will be determined internally during a search of standard
   * configuration file locations.
   *
   * @param configfile short name of the config file
   */
  ConfigurationManager(const std::string& configfile);

  /**
   * Initialize the configuration manager to its default settings. This method
   * may be called from the child class's constructor to initialize its values.
   */
  virtual void init() = 0;

  /**
   * Decode values from the Json tree, and assign them to your options. Use the
   * following macros to decode values:
   *   CM_DecodeString()
   *   CM_DecodeInt()
   *   CM_DecodeUInt()
   *   CM_DecodeDouble()
   *   CM_DecodeBool()
   *
   * @param root Json option tree's root node
   */
  virtual void decode(const Json::Value& root) = 0;

private:
  /** Configuration file short name */
  const std::string configfile;
};

#define CM_DecodeString(root, optionName, optionVariable) \
  do { \
    if(root[optionName].isString()) { \
      optionVariable = root[optionName].asString(); \
    } else { \
      LOG_ERROR(optionName " is missing or wrong type (should be string)"); \
    } \
  } while (0)

#define CM_DecodeInt(root, optionName, optionVariable) \
  do { \
    if(root[optionName].isString()) { \
      optionVariable = root[optionName].asInt(); \
    } else { \
      LOG_ERROR(optionName " is missing or wrong type (should be integer)"); \
    } \
  } while (0)

#define CM_DecodeUInt(root, optionName, optionVariable) \
  do { \
    if(root[optionName].isString()) { \
      optionVariable = root[optionName].asUInt(); \
    } else { \
      LOG_ERROR(optionName " is missing or wrong type (should be unsigned integer)"); \
    } \
  } while (0)

#define CM_DecodeDouble(root, optionName, optionVariable) \
  do { \
    if(root[optionName].isString()) { \
      optionVariable = root[optionName].asDouble(); \
    } else { \
      LOG_ERROR(optionName " is missing or wrong type (should be double)"); \
    } \
  } while (0)

#define CM_DecodeBool(root, optionName, optionVariable) \
  do { \
    if(root[optionName].isString()) { \
      optionVariable = root[optionName].asBool(); \
    } else { \
      LOG_ERROR(optionName " is missing or wrong type (should be boolean)"); \
    } \
  } while (0)


#include <fstream>
#include <ace/OS_NS_stdlib.h>
#include <ace/OS_NS_sys_stat.h>

#include "json/reader.h"

#ifdef WIN32
#include <Windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif

#include "log.h"

#define CM_CONFIG_DIRECTORY "ammo-gateway"

inline ConfigurationManager::ConfigurationManager(const std::string& configfile)
  : configfile(configfile)
{
}

#ifndef WIN32
inline string ConfigurationManager::findConfigFile(const std::string& configfile) {
  std::string filePath;
  ACE_stat statStruct;
  const char* CONFIG_FILE = configfile.c_str();
  
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
  
  filePath = CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + CM_CONFIG_DIRECTORY + "/" + CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = std::string("/etc/") + CM_CONFIG_DIRECTORY + "/" + CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = std::string("../etc/") + CONFIG_FILE;
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
#else
inline std::string ConfigurationManager::findConfigFile(const std::string& configfile) {
  std::string filePath;
  ACE_stat statStruct;
  const char* CONFIG_FILE = configfile.c_str();
  
  std::string gatewayRoot;
  
  TCHAR userConfigPath[MAX_PATH];
  TCHAR systemConfigPath[MAX_PATH];

  SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, userConfigPath);
  SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, systemConfigPath);
  
  char *gatewayRootC = ACE_OS::getenv("GATEWAY_ROOT");
  if(gatewayRootC == NULL) {
    gatewayRoot = "";
  } else {
    gatewayRoot = gatewayRootC;
  }
  
  filePath = CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = std::string(userConfigPath) + "\\" + CM_CONFIG_DIRECTORY + "\\" + CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = std::string(systemConfigPath) + "\\" + CM_CONFIG_DIRECTORY + "\\" + CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "\\etc\\" + CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "\\build\\etc\\" + CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = std::string("..\\etc\\") + CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No config file found.");
              return std::string();
            }
          }
        }
      }
    }
  }
  
  LOG_INFO("Using config file: " << filePath);
  return filePath;
}
#endif

inline void ConfigurationManager::populate()
{
  this->init();

  LOG_DEBUG("Parsing config file...");
  std::string configFilename = findConfigFile();
  
  if(configFilename != "") {
    ifstream configFile(configFilename.c_str());
    if(configFile) {
      Json::Value root;
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse(configFile, root);
      
      if(parsingSuccessful) {
        decode(root);
      } else {
        LOG_ERROR("JSON parsing error in config file '" << this->configfile << "'.  Using defaults.");
      }
    } else {
      LOG_WARN("Could not read from config file '" << this->configfile << "'.  Using defaults.");
    }
  } else {
    LOG_WARN("Using default configuration.");
  }
}

inline std::string ConfigurationManager::findConfigFile()
{
  return ConfigurationManager::findConfigFile(this->configfile);
}

#endif //CONFIGURATION_MANAGER_H
