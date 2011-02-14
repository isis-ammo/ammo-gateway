#ifndef PASS_AMMO_PUBLISHER_H
#define PASS_AMMO_PUBLISHER_H

#include <string>
#include <map>

#include "GatewayConnector.h"

class BSO {
public:
  std::string uuid;
  ACE_Time_Value updateTime;
  double latitude;
  double longitude;
};

class PassAmmmoPublisher {
public:
  static GatewayConnector *connector;
  
  static std::map<std::string, BSO> recentObjects;
  
  static void pushBso(std::string id, std::string name, std::string symbolCode,
    std::string service, std::string latitude, std::string longitude,
    std::string reportDate, std::string altitude);
};

double latLongDistance(double startLatitude, double startLongitude, double endLatitude, double endLongitude, double &outAzimuth);

#endif // PASS_AMMO_PUBLISHER_H
