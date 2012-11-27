#include <string>
#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

#include "ace/Signal.h"
#include <ace/OS_NS_sys_stat.h>
#include <ace/Logging_Strategy.h>

#ifdef WIN32
#include <Windows.h>
#include <shlwapi.h>
#include <ShlObj.h>
#endif

const char *LOG_CONFIG_DIRECTORY = "ammo-gateway";
const char *LOG_CONFIG_FILE = "LoggingConfig.json";

const char *TIME_FORMAT = "%Y%m%dT%H%M%SZ";

std::string findConfigFile();
std::string expandLogFileName(std::string fileName, std::string appName);

/**
* This function reads the logging config file and sets up logging accordingly.
* This should be included in main.cpp and called exactly once when the
* application is initialized.
*/
void setupLogging(std::string appName) {
  std::string configFilename = findConfigFile();
  
  std::string logLevel = "trace";
  std::string logFile = "";
  int logFileMaxSize = 10000;
  int logFileSampleTime = 30;
  int logFileCount = 20;
  
  if(configFilename != "") {
    std::ifstream configFile(configFilename.c_str());
    if(configFile) {
      Json::Value root;
      Json::Reader reader;
      
      bool parsingSuccessful = reader.parse(configFile, root);
      
      if(parsingSuccessful) {
        if(root["LogLevel"].isString()) {
          logLevel = root["LogLevel"].asString();
          std::transform(logLevel.begin(), logLevel.end(), logLevel.begin(), tolower);
        } else {
          LOG_ERROR("LogLevel is missing or wrong type (should be string)");
        }
        
        if(root["LogFile"].isString()) {
          logFile = root["LogFile"].asString();
        } else {
          //don't write an error; having the LogFile parameter unconfigured is a
          //valid scenario
        }
        
        if(root["LogFileMaxSize"].isInt()) {
          /**
          * Log file maximum size in KB
          * 0 means don't rotate (maintain old behavior)
          */
          logFileMaxSize = root["LogFileMaxSize"].asInt();
        } else {
          LOG_ERROR("LogFileMaxSize is missing or wrong type (should be int)");
        }
        
        if(root["LogFileSampleTime"].isInt()) {
          int temp = root["LogFileSampleTime"].asInt();
          if(temp <= 0) {
            LOG_ERROR("Invalid LogFileSampleTime (must be > 0)");
          } else {
            logFileSampleTime = temp;
          }
        } else {
          LOG_ERROR("LogFileSampleTime is missing or wrong type (should be int)");
        }
        
        if(root["LogFileCount"].isInt()) {
          int temp = root["LogFileCount"].asInt();
          if(temp <= 0) {
            LOG_ERROR("Invalid LogFileCount (must be > 0)");
          } else {
            logFileCount = temp;
          }
        } else {
          LOG_ERROR("LogFileCount is missing or wrong type (should be int)");
        }
        
        
      } else {
        LOG_ERROR("JSON parsing error in config file '" << LOG_CONFIG_FILE << "'.  Using defaults.");
      }
    } else {
      LOG_WARN("Could not read from config file '" << LOG_CONFIG_FILE << "'.  Using defaults.");
    }
  } else {
    LOG_WARN("Using default configuration.");
  }
  
  if(logFile != "") { //blank filename or no logFile entry in config file writes to stderr
    if(logFileMaxSize > 0) {
      std::string expandedFilename = expandLogFileName(logFile, appName);
      LOG_INFO("Logging to logrolled file with base name " << expandedFilename);
      
      std::ostringstream maxSizeStringStream;
      maxSizeStringStream << logFileMaxSize;
      std::string maxSizeString = maxSizeStringStream.str();
      
      std::ostringstream sampleTimeStringStream;
      sampleTimeStringStream << logFileSampleTime;
      std::string sampleTimeString = sampleTimeStringStream.str();
      
      std::ostringstream countStringStream;
      countStringStream << logFileCount;
      std::string countString = countStringStream.str();
      
      LOG_INFO("Logging to file " << expandedFilename);
      ACE_TCHAR *l_argv[6]; 
      l_argv[0] = (ACE_TCHAR *) ACE_TEXT ("-s"); //log file basename 
      l_argv[1] = (ACE_TCHAR *) ACE_TEXT (expandedFilename.c_str());
      l_argv[1] = (ACE_TCHAR *) ACE_TEXT ("-o");      //order files 
      l_argv[2] = (ACE_TCHAR *) ACE_TEXT ("-m");  //max size = ~1MB (1000 KB) 
      l_argv[1] = (ACE_TCHAR *) ACE_TEXT (maxSizeString.c_str()); 
      l_argv[3] = (ACE_TCHAR *) ACE_TEXT ("-i");    //logfile size is sampled every 30 seconds 
      l_argv[1] = (ACE_TCHAR *) ACE_TEXT (sampleTimeString.c_str());
      l_argv[4] = (ACE_TCHAR *) ACE_TEXT ("-N");     //support 2 files 
      l_argv[4] = (ACE_TCHAR *) ACE_TEXT (countString.c_str());
      l_argv[5] = 0;
      
      int ls_argc = 5; 
      ACE_Auto_Basic_Ptr<ACE_TCHAR *> ls_argv (new ACE_TCHAR *[ls_argc]); 
      for (int c = 0; c < ls_argc; ++c) 
              (ls_argv.get ())[c] = l_argv[c];
      
      //Allocate the logging strategy on the heap, because if it's stack allocated,
      //it will be deleted when it goes out of scope and horrible things will happen
      //(yes, that means we leak it)
      ACE_Logging_Strategy *logging_strategy = new ACE_Logging_Strategy(); 
      int result = logging_strategy->init (ls_argc, ls_argv.get ());
      
      if(result != 0) {
        LOG_WARN("Error creating logging strategy and/or log file...  does the directory exist?");
        LOG_WARN("Logging to console instead.");
      }
    } else {
      std::string expandedFilename = expandLogFileName(logFile, appName);
      LOG_INFO("Logging to single file " << expandedFilename);
      ACE_OSTREAM_TYPE *output = new std::ofstream(expandedFilename.c_str(), std::ofstream::out | std::ofstream::app);
      if(*output) {
        ACE_LOG_MSG->msg_ostream(output, 1);
        ACE_LOG_MSG->set_flags(ACE_Log_Msg::OSTREAM);
        ACE_LOG_MSG->clr_flags(ACE_Log_Msg::STDERR);
      } else {
        LOG_WARN("Couldn't create log file...  does the directory exist?");
        LOG_WARN("Logging to console instead.");
      }
    }
  }
  
  if(logLevel == "trace") {
    ACE_LOG_MSG->priority_mask (LM_TRACE | LM_DEBUG | LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "debug") {
    ACE_LOG_MSG->priority_mask (LM_DEBUG | LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "info") {
    ACE_LOG_MSG->priority_mask (LM_INFO | LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "warning") {
    ACE_LOG_MSG->priority_mask (LM_WARNING | LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "error") {
    ACE_LOG_MSG->priority_mask (LM_ERROR | LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else if(logLevel == "critical") {
    ACE_LOG_MSG->priority_mask (LM_CRITICAL, ACE_Log_Msg::PROCESS);
  } else {
    LOG_ERROR("Unknown logging level... using default configuration.");
  }
  
  
}

/*
* This formats the log file name using a simplified printf-like syntax:
*   %%: Literal percent sign
*   %A: Application name (parameter to this function)
*   %T: Current ISO 8601 date and time (UTC) (YYYYMMDDThhmmssZ; i.e. 20120105T065312Z)
*
* Unknown format specifiers will be removed.
*/
std::string expandLogFileName(std::string logFile, std::string appName) {
  std::ostringstream outputFilename;
  
  time_t rawTime;
  struct tm *utcTime;
  char utcTimeFormatted[40];
  
  time(&rawTime);
  utcTime = gmtime(&rawTime);
  int len = strftime(utcTimeFormatted, 40, TIME_FORMAT, utcTime);
  if(len == 0) { //didn't work (not big enough); contents of array are indeterminate, so we need to put something in it
    utcTimeFormatted[0] = '\0';
  }
  
  for(std::string::iterator it = logFile.begin(); it != logFile.end(); it++) {
    if((*it) == '%') {
      it++;
      char formatCharacter = *it;
      switch(formatCharacter) {
      case '%':
        outputFilename << '%';
        break;
      case 'A':
        outputFilename << appName;
        break;
      case 'T':
        outputFilename << utcTimeFormatted;
        break;
      }
    } else {
      outputFilename << (*it);
    }
  }
  return outputFilename.str();
}

#ifndef WIN32
std::string findConfigFile() {
  std::string filePath;
  ACE_stat statStruct;
  
  std::string home, gatewayRoot;
  
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
  
  filePath = LOG_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = home + "/" + "." + LOG_CONFIG_DIRECTORY + "/" + LOG_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = std::string("/etc/") + LOG_CONFIG_DIRECTORY + "/" + LOG_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "/etc/" + LOG_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "/build/etc/" + LOG_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = std::string("../etc/") + LOG_CONFIG_FILE;
            if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
              LOG_ERROR("No logging config file found.");
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
/**
 * Searches for the gateway config file.  Search order:
 *   1) The current working directory
 *   2) The user's configuration directory (Roaming appdata directory/ammo-gateway)
 *   3) The all users configuration directory (i.e. C:\ProgramData\ammo-gateway on Vista/7)
 *   Fallback locations (don't rely on these; they may change or disappear in a
 *   future release.  Gateway installation should put the config file into
 *   a location that's searched by default):
 *   4) $GATEWAY_ROOT/etc
 *   5) $GATEWAY_ROOT/build/etc
 *   6) ../etc
 */
std::string findConfigFile() {
  std::string filePath;
  ACE_stat statStruct;
  
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
  
  filePath = LOG_CONFIG_FILE;
  //stat returns 0 if the file exists
  if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
    filePath = std::string(userConfigPath) + "\\" + LOG_CONFIG_DIRECTORY + "\\" + LOG_CONFIG_FILE;
    if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
      filePath = std::string(systemConfigPath) + "\\" + LOG_CONFIG_DIRECTORY + "\\" + LOG_CONFIG_FILE;
      if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
        filePath = gatewayRoot + "\\etc\\" + LOG_CONFIG_FILE;
        if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
          filePath = gatewayRoot + "\\build\\etc\\" + LOG_CONFIG_FILE;
          if(ACE_OS::stat(filePath.c_str(), &statStruct)) {
            filePath = std::string("..\\etc\\") + LOG_CONFIG_FILE;
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
#endif


//:mode=c++: (jEdit modeline for syntax highlighting)
