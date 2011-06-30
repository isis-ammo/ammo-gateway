#ifndef DATASTORE_CONFIG_MANAGER_H
#define DATASTORE_CONFIG_MANAGER_H

#include "json/reader.h"
#include "json/value.h"

class DataStoreReceiver;

namespace ammo
{
  namespace gateway
  {
    class GatewayConnector;
  }
}

class DataStoreConfigManager
{
public:
  static
  DataStoreConfigManager *getInstance (
	DataStoreReceiver *receiver,
	ammo::gateway::GatewayConnector *connector);
	
private:
  DataStoreConfigManager (
	DataStoreReceiver *receiver,
	ammo::gateway::GatewayConnector *connector);
	
  std::string findConfigFile ();
	
  static DataStoreConfigManager *sharedInstance;
	
  Json::Value root_;
	
  DataStoreReceiver *receiver_;
  ammo::gateway::GatewayConnector *connector_;
};

#endif // DATASTORE_CONFIG_MANAGER_H
