#include <sstream>
#include <string>

#include "log.h"
#include "GatewayConnector.h"

#include "PassAmmmoPublisher.h"
#include "PassConfigurationManager.h"

using namespace std;
using namespace ammo::gateway;

GatewayConnector *PassAmmmoPublisher::connector = 0;

void
PassAmmmoPublisher::pushPli (const string &lid,
                             const string &userid,
                             const string &unitid,
                             const string &name,
                             const string &lat,
                             const string &lon,
                             const string &created,
                             const string &modified)
{
  ostringstream json;
  json << "{";
  json << "\"lid\": \"" << lid << "\", ";
  json << "\"userid\": \"" << userid << "\", ";
  json << "\"unitid\": \"" << unitid << "\", ";
  json << "\"name\": \"" << name << "\", ";
  json << "\"lat\": \"" << lat << "\", ";
  json << "\"lon\": \"" << lon << "\",";
  json << "\"created\": \"" << created << "\", ";
  json << "\"modified\": \"" << modified << "\"";
  json << "}";
  
  PushData pd;
  
  // This field isn't used by the gateway, but we set it anyway
  // to some non-empty semi-informative value.
  pd.uri = "nevada:locations";
  
  pd.mimeType =
    PassConfigurationManager::getInstance ()->getPassContentTopic ();
  pd.data = json.str ();
  
  if (connector != 0)
    {
      connector->pushData (pd);
    }
  else
    {
      LOG_ERROR ("PassAmmmoPublisher::pushPli - "
                 "connector not initialized");
    }
}

