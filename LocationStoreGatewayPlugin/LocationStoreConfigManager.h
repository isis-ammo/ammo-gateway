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
	LocationStoreReceiver *push_receiver,
	GatewayConnector *connector);
	
private:
  LocationStoreConfigManager (
	LocationStoreReceiver *push_receiver,
	GatewayConnector *connector);
	
  static LocationStoreConfigManager *sharedInstance;
	
  Json::Value root;
	
  LocationStoreReceiver *receiver;
  GatewayConnector *connector;
};

#endif // LOCATIONSTORE_CONFIG_MANAGER_H
