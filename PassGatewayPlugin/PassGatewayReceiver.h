#ifndef PASS_GATEWAY_RECEIVER_H
#define PASS_GATEWAY_RECEIVER_H

#include "GatewayConnector.h"

class PassConfigurationManager;

namespace ACE_Utils
{
  class UUID;
}

class PassGatewayReceiver : public ammo::gateway::DataPushReceiverListener,
					                  public ammo::gateway::GatewayConnectorDelegate

{
public:
  PassGatewayReceiver (void);
  ~PassGatewayReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect (ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived (ammo::gateway::GatewayConnector *sender,
							                     ammo::gateway::PushData &pushData);
							                     
  void set_cfg_mgr (PassConfigurationManager * const cfg_mgr);
  
private:
  PassConfigurationManager *cfg_mgr_;
  std::string plugin_id_;
};

#endif // #ifndef PASS_GATEWAY_RECEIVER_H

