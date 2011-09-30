#include <sstream>
#include <string>

#include "log.h"
#include "GatewayConnector.h"

#include "PassAmmmoPublisher.h"
#include "PassConfigurationManager.h"

using namespace std;
using namespace ammo::gateway;

GatewayConnector *PassAmmmoPublisher::connector = 0;

static int 
userToId(const string &user) {
  int uid = 0;
  for(int i=0; i<user.size(); i++) uid += user[i];
  return uid;
}

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
  // Verify this --
  // PASS sends lat/lon in decimal degrees, BFT expects degrees * 1.0E6
  // PASS sends date as ASCII string - zulu time, BFT expects seconds since epoch
  // we need to turn user id and unit id into numbers


  long  llat = (long)(strtod(lat.c_str(), 0) * 1.0E6);
  long  llon = (long)(strtod(lon.c_str(), 0) * 1.0E6);

  struct tm creatm;
  strptime (created.c_str(), "%Y-%m-%dT%H:%M:%SZ", &creatm);
  time_t creat = mktime(&creatm);

  


  ostringstream json;
  json << "{";
  json << "\"lid\": \"" << lid << "\", ";
  json << "\"userid\": \"" << userToId(userid) << "\", "; 
  json << "\"unitid\": \"" << 0 << "\", ";   // map from unitid string
  json << "\"name\": \"" << name << "\", ";
  json << "\"lat\": \"" << llat << "\", ";
  json << "\"lon\": \"" << llon << "\", ";
  json << "\"created\": \"" << creat * 1000ll << "\", ";
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

