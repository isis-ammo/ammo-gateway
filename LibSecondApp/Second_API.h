#ifndef SECOND_API_H
#define SECOND_API_H

#include "DataStore_API.h"
#include "Second_API_Export.h"

class Second_API_Export Second_API : public DataStore_API
{
public:
  Second_API (void);
  virtual ~Second_API (void);

  virtual void insert (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData);

  virtual void update (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PushData &pushData);

  virtual void query (ammo::gateway::GatewayConnector *sender,
                      const ammo::gateway::PullRequest &pullReq);

  virtual void remove (ammo::gateway::GatewayConnector *sender,
							         const ammo::gateway::PullRequest &pullReq);
};

extern "C" Second_API_Export DataStore_API *create_Second_API (void);

#endif // SECOND_API_H
