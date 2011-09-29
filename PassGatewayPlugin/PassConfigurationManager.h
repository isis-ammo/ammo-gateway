#ifndef PASS_CONFIGURATION_MANAGER_H
#define PASS_CONFIGURATION_MANAGER_H

#include <string>

class PassGatewayReceiver;

namespace ammo
{
  namespace gateway
  {
    class GatewayConnector;
  }
}

class PassConfigurationManager
{
public:
  static PassConfigurationManager*
  getInstance (PassGatewayReceiver *receiver = 0,
               ammo::gateway::GatewayConnector *connector = 0);
  
  const std::string &getPassServerAddress (void) const;
  const std::string &getPassTopic (void) const;
  const std::string &getPassSubscriberId (void) const;
  const std::string &getPassPublisherId (void) const;
  const std::string &getPassSubscriberInterface (void) const;
  int getPassSubscriberPort (void) const;
  const std::string &getPassSubscriberAddress (void) const;
  
  const std::string &getPassPluginId (void) const;
  
  PassGatewayReceiver *getReceiver (void) const;
  
private:
  PassConfigurationManager (PassGatewayReceiver *receiver,
                            ammo::gateway::GatewayConnector *connector);

  std::string findConfigFile (void);
	
private:
  static PassConfigurationManager *sharedInstance;
  
  std::string passServerAddress;
  std::string passTopic;
  std::string passSubscriberId;
  std::string passPublisherId;
  std::string passSubscriberInterface;
  int passSubscriberPort;
  std::string passSubscriberAddress;
  
  // Generated unique id used as a prefix to the id of each SOAP publish
  // originating locally. When our subscriber gets the message back,
  // we can use it to short-circuit the local data push.
  std::string passPluginId;
  
  PassGatewayReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
};

#endif //PASS_CONFIGURATION_MANAGER_H
