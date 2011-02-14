#ifndef PASS_CONFIGURATION_MANAGER_H
#define PASS_CONFIGURATION_MANAGER_H

#include <string>

class PassConfigurationManager {
public:
  static PassConfigurationManager* getInstance();
  
  std::string getPassServerAddress();
  std::string getPassTopic();
  std::string getPassSubscriberId();
  std::string getPassSubscriberInterface();
  int getPassSubscriberPort();
  std::string getPassSubscriberAddress();
private:
  PassConfigurationManager();
  static PassConfigurationManager *sharedInstance;
  
  std::string passServerAddress;
  std::string passTopic;
  std::string passSubscriberId;
  std::string passSubscriberInterface;
  int passSubscriberPort;
  std::string passSubscriberAddress;
};

#endif //PASS_CONFIGURATION_MANAGER_H
