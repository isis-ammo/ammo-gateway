#ifndef APP_API_H
#define APP_API_H

#include "DataStore_API.h"
#include "App_API_Export.h"

class App_API_Export App_API : public DataStore_API
{
public:
  App_API (void);
  virtual ~App_API (void);

  virtual void insert (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData);

  virtual void update (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData);

  virtual void query (ammo::gateway::GatewayConnector *sender,
                      const ammo::gateway::PullRequest &pullReq);

  virtual void remove (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PullRequest &pullReq);
};

extern "C" App_API_Export DataStore_API *create_App_API (void);

#endif // APP_API_H
