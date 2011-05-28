#ifndef LOCATIONSTORE_CONFIG_MANAGER_H
#define LOCATIONSTORE_CONFIG_MANAGER_H

#include "json/reader.h"
#include "json/value.h"

class LocationStoreReceiver;
namespace ammo {
  namespace gateway {
    class GatewayConnector;
  }
}

class LocationStoreConfigManager
{
public:
  static
  LocationStoreConfigManager *getInstance (
	LocationStoreReceiver *receiver,
	ammo::gateway::GatewayConnector *connector);
	
private:
  LocationStoreConfigManager (
	LocationStoreReceiver *receiver,
	ammo::gateway::GatewayConnector *connector);
	
  std::string findConfigFile ();
	
  static LocationStoreConfigManager *sharedInstance;
	
  Json::Value root_;
	
  LocationStoreReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
};

#endif // LOCATIONSTORE_CONFIG_MANAGER_H
