#include <iostream>

#include "Second_API.h"

using namespace std;

Second_API::Second_API (void)
{
}

Second_API::~Second_API (void)
{
}

void
Second_API::insert (ammo::gateway::GatewayConnector * /* sender */,
							   const ammo::gateway::PushData & /* pushData */)
{
  cout << "calling second insert" << endl;
}

void
Second_API::update (ammo::gateway::GatewayConnector * /* sender */,
							   const ammo::gateway::PushData & /* pushData */)
{
  cout << "calling second update" << endl;
}

void
Second_API::query (ammo::gateway::GatewayConnector * /* sender */,
                const ammo::gateway::PullRequest & /* pullReq */)
{
  cout << "calling second query" << endl;
}

void
Second_API::remove (ammo::gateway::GatewayConnector * /* sender */,
                 const ammo::gateway::PullRequest & /* pullReq */)
{
  cout << "calling second remove" << endl;
}

extern "C" Second_API_Export DataStore_API *create_Second_API (void)
{
  return new Second_API;
}
