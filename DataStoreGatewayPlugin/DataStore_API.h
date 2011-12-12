#ifndef DATASTORE_API_H
#define DATASTORE_API_H

#include "GatewayConnector.h"

class DataStore_API
{
public:
  DataStore_API (void) {}
  virtual ~DataStore_API (void) {}

  virtual void insert (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData) = 0;

  virtual void update (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData) = 0;

  virtual void query (ammo::gateway::GatewayConnector *sender,
                      const ammo::gateway::PullRequest &pullReq) = 0;

  virtual void remove (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PullRequest &pullReq) = 0;
};

// Declare the type of the factory function pointer.
typedef DataStore_API *(*API_Factory)(void);

#endif // DATASTORE_API_H
