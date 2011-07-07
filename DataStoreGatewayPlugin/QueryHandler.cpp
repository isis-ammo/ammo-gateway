#include "json/reader.h"
#include "json/value.h"

#include "log.h"

#include "QueryHandler.h"

QueryHandler::QueryHandler (sqlite3 *db,
                            ammo::gateway::GatewayConnector *sender,
                            ammo::gateway::PullRequest &pr)
  : db_ (db),
    sender_ (sender),
    pr_ (pr)
{
}

QueryHandler::~QueryHandler (void)
{
}

bool
QueryHandler::parseJson (const std::string &input,
                         Json::Value& root)
{
  Json::Reader jsonReader;
  bool parseSuccess = jsonReader.parse (input, root);

  if (!parseSuccess)
    {
      LOG_ERROR ("JSON parsing error:"
                 << jsonReader.getFormatedErrorMessages ());
                 
      return parseSuccess;
    }

  LOG_DEBUG ("Parsed JSON: " << root.toStyledString ());
  return parseSuccess;
}

