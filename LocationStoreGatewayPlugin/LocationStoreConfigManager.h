#ifndef LOCATIONSTORE_CONFIG_MANAGER_H
#define LOCATIONSTORE_CONFIG_MANAGER_H

#include "json/reader.h"
#include "json/value.h"

class LocationStoreReceiver;
class GatewayConnector;

class LocationStoreConfigManager
{
public:
  static
  LocationStoreConfigManager *getInstance (
	LocationStoreReceiver *receiver,
	GatewayConnector *connector);
	
private:
  LocationStoreConfigManager (
	LocationStoreReceiver *receiver,
	GatewayConnector *connector);
	
  std::string findConfigFile ();
	
  static LocationStoreConfigManager *sharedInstance;
	
  Json::Value root_;
	
  LocationStoreReceiver *receiver_;
  GatewayConnector *connector_;
};

#endif // LOCATIONSTORE_CONFIG_MANAGER_H
