#include <iostream>

#include "App_API.h"

using namespace std;

App_API::App_API (void)
{
}

App_API::~App_API (void)
{
}

void
App_API::insert (ammo::gateway::GatewayConnector * /* sender */,
							   const ammo::gateway::PushData & /* pushData */)
{
  cout << "calling first insert" << endl;
}

void
App_API::update (ammo::gateway::GatewayConnector * /* sender */,
							   const ammo::gateway::PushData & /* pushData */)
{
  cout << "calling first update" << endl;
}

void
App_API::query (ammo::gateway::GatewayConnector * /* sender */,
                const ammo::gateway::PullRequest & /* pullReq */)
{
  cout << "calling first query" << endl;
}

void
App_API::remove (ammo::gateway::GatewayConnector * /* sender */,
                 const ammo::gateway::PullRequest & /* pullReq */)
{
  cout << "calling first remove" << endl;
}

extern "C" App_API_Export DataStore_API *create_App_API (void)
{
  return new App_API;
}
