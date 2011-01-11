#include "PassAmmmoPublisher.h"

#include <string>
#include <sstream>
#include <iostream>
#include <cmath>

using namespace std;

const string BSO_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.battlespace.gcm";

const double SEMI_MAJOR_AXIS = 6378137.0; //WGS84 in meters
const double SEMI_MINOR_AXIS = 6356752.314245 ;
const double FLATTENING = 1 / 298.257223563;

GatewayConnector *PassAmmmoPublisher::connector = NULL;
std::map<std::string, BSO> PassAmmmoPublisher::recentObjects;

void PassAmmmoPublisher::pushBso(std::string id, std::string name, std::string symbolCode,
    std::string service, std::string latitude, std::string longitude,
    std::string reportDate, std::string altitude) {
  //get most recent latitude and longitude for this object, if it exists
  double speed = 0.0;
  double bearing = 0.0;
  double lat = atof(latitude.c_str());
  double lon = atof(longitude.c_str());
  
  map<string, BSO>::iterator it = recentObjects.find(id);
  if(it != recentObjects.end()) {
    double distanceMeters = latLongDistance(it->second.latitude, it->second.longitude, lat, lon, bearing);
    
    unsigned long long oldTimeMilliseconds = it->second.updateTime.get_msec();
    ACE_Time_Value newTime = ACE_OS::gettimeofday();
    unsigned long long newTimeMilliseconds = newTime.get_msec();
    unsigned long long millisecDifference = newTimeMilliseconds - oldTimeMilliseconds;
    if(millisecDifference != 0) {
      double speedMetersPerMillisecond = distanceMeters / millisecDifference;
      speed = speedMetersPerMillisecond / 1000; //meters per second
    }
    cout << "Old object with UUID " << id << " found; speed: " << speed << ", bearing: " << bearing << endl << flush;
    it->second.latitude = lat;
    it->second.longitude = lon;
    it->second.updateTime = newTime;
    
    if(isnan(bearing)) {
      bearing = 0.0;
    }
  } else {
    BSO newObject;
    newObject.uuid = id;
    newObject.latitude = lat;
    newObject.longitude = lon;
    newObject.updateTime = ACE_OS::gettimeofday();
    recentObjects[newObject.uuid] = newObject;
  }

  //build the JSON string.
  //TODO: pass through created date from the PASS message; decompose the symbol code?
  ostringstream json;
  json << "{";
  json << "\"uuid\": \"" << id << "\", ";
  json << "\"longitude\": " << longitude << ", ";
  json << "\"latitude\": " << latitude << ", ";
  json << "\"sidc\": \"" << symbolCode << "\",";
  json << "\"speed\": " << speed << ", ";
  json << "\"heading\": " << bearing << "";
  json << "}";
  cout << json.str() << endl << flush;
  
  ostringstream uri;
  uri << "bft:bso/" << id;
  if(connector) {
    connector->pushData(uri.str(), BSO_MIME_TYPE, json.str());
  }
}

//converted from C# code from http://www.codeproject.com/KB/cs/Vincentys_Formula.aspx
double latLongDistance(double startLatitude, double startLongitude, double endLatitude, double endLongitude, double &outAzimuth) {
  double startLatitudeRadians = startLatitude * (M_PI/180);
  double startLongitudeRadians = startLongitude * (M_PI/180);
  
  double endLatitudeRadians = endLatitude * (M_PI/180);
  double endLongitudeRadians = endLongitude * (M_PI/180);
  
  // get constants
  double a = SEMI_MAJOR_AXIS;
  double b = SEMI_MINOR_AXIS;
  double f = FLATTENING;

  // get parameters as radians
  double phi1 = startLatitudeRadians;
  double lambda1 = startLongitudeRadians;
  double phi2 = endLatitudeRadians;
  double lambda2 = endLongitudeRadians;

  // calculations
  double a2 = a * a;
  double b2 = b * b;
  double a2b2b2 = (a2 - b2) / b2;

  double omega = lambda2 - lambda1;

  double tanphi1 = tan(phi1);
  double tanU1 = (1.0 - f) * tanphi1;
  double U1 = atan(tanU1);
  double sinU1 = sin(U1);
  double cosU1 = cos(U1);

  double tanphi2 = tan(phi2);
  double tanU2 = (1.0 - f) * tanphi2;
  double U2 = atan(tanU2);
  double sinU2 = sin(U2);
  double cosU2 = cos(U2);

  double sinU1sinU2 = sinU1 * sinU2;
  double cosU1sinU2 = cosU1 * sinU2;
  double sinU1cosU2 = sinU1 * cosU2;
  double cosU1cosU2 = cosU1 * cosU2;

  // eq. 13
  double lambda = omega;

  // intermediates we'll need to compute 's'
  double A = 0.0;
  double B = 0.0;
  double sigma = 0.0;
  double deltasigma = 0.0;
  double lambda0;
  bool converged = false;

  for (int i = 0; i < 10; i++)
  {
      lambda0 = lambda;

      double sinlambda = sin(lambda);
      double coslambda = cos(lambda);

      // eq. 14
      double sin2sigma = (cosU2 * sinlambda * cosU2 * sinlambda) +
           (cosU1sinU2 - sinU1cosU2 * coslambda) *
           (cosU1sinU2 - sinU1cosU2 * coslambda);

      double sinsigma = sqrt(sin2sigma);

      // eq. 15
      double cossigma = sinU1sinU2 + (cosU1cosU2 * coslambda);

      // eq. 16
      sigma = atan2(sinsigma, cossigma);

      // eq. 17 Careful! sin2sigma might be almost 0!
      double sinalpha = (sin2sigma == 0) ? 0.0 :
            cosU1cosU2 * sinlambda / sinsigma;

      double alpha = asin(sinalpha);
      double cosalpha = cos(alpha);
      double cos2alpha = cosalpha * cosalpha;

      // eq. 18 Careful! cos2alpha might be almost 0!
      double cos2sigmam = cos2alpha == 0.0 ? 0.0 :
          cossigma - 2 * sinU1sinU2 / cos2alpha;

      double u2 = cos2alpha * a2b2b2;

      double cos2sigmam2 = cos2sigmam * cos2sigmam;

      // eq. 3
      A = 1.0 + u2 / 16384 * (4096 + u2 *
          (-768 + u2 * (320 - 175 * u2)));

      // eq. 4
      B = u2 / 1024 * (256 + u2 * (-128 + u2 * (74 - 47 * u2)));

      // eq. 6
      deltasigma = B * sinsigma * (cos2sigmam + B / 4
          * (cossigma * (-1 + 2 * cos2sigmam2) - B / 6
          * cos2sigmam * (-3 + 4 * sin2sigma)
          * (-3 + 4 * cos2sigmam2)));

      // eq. 10
      double C = f / 16 * cos2alpha * (4 + f * (4 - 3 * cos2alpha));

      // eq. 11 (modified)
      lambda = omega + (1 - C) * f * sinalpha
          * (sigma + C * sinsigma * (cos2sigmam + C
          * cossigma * (-1 + 2 * cos2sigmam2)));

      // see how much improvement we got
      double change = abs((lambda - lambda0) / lambda);

      if ((i > 1) && (change < 0.0000000000001))
      {
           converged = true;
           break;
      }
  }

  // eq. 19
  double s = b * A * (sigma - deltasigma);
  double alpha1;
  double alpha2;

  // didn't converge? must be N/S
  if (!converged)
  {
      if (phi1 > phi2)
      {
          alpha1 = M_PI;
          alpha2 = 0;
      }
      else if (phi1 < phi2)
      {
          alpha1 = 0;
          alpha2 = M_PI;
      }
      else
      {
          alpha1 = numeric_limits<double>::quiet_NaN();
          alpha2 = numeric_limits<double>::quiet_NaN();
      }
  }
  // else, it converged, so do the math
  else
  {
      double radians;
      alpha1 = 0;
      alpha2 = 0;

      // eq. 20
      radians = atan2(cosU2 * sin(lambda),
      (cosU1sinU2 - sinU1cosU2 * cos(lambda)));

      if (radians < 0.0) radians += (2*M_PI);
      alpha1 = radians;

      // eq. 21
      radians = atan2(cosU1 * sin(lambda),
              (-sinU1cosU2 + cosU1sinU2 *
      cos(lambda))) + M_PI;

      if (radians < 0.0) radians += (2*M_PI);
      alpha2 = radians;
  }
  
  outAzimuth = alpha1 * (180/M_PI); //direction in degrees from start to end (alpha2 is end to start direction in radians) 

  return s;
}
