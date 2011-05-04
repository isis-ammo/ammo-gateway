#include "GatewayConnector.h"

#include "SpotPushReceiver.h"

#include "json/json.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "mgrs.h"


//#include <boost/tokenizer.hpp>
//#include <boost/foreach.hpp>
//#include <boost/algorithm/string.hpp>

int spot_id = 1000;

using namespace std;
using namespace ammo::gateway;

const string BSO_MIME_TYPE = "application/vnd.edu.vu.isis.ammo.battlespace.gcm";

void SpotPushReceiver::onConnect(GatewayConnector *sender) {
  
}

void SpotPushReceiver::onDisconnect(GatewayConnector *sender) {
  
}

void SpotPushReceiver::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  cout << "Got data." << endl;
  cout << "  URI: " << pushData.uri << endl;
  cout << "  Mime type: " << pushData.mimeType << endl;
  cout << "  Data: " << string(pushData.data.begin(), pushData.data.end()) << endl;
  cout << "  Origin Username: " << pushData.originUsername << endl;

    cout << "Extracting JSON metadata..." << endl << flush;
    
    unsigned int jsonEnd = 0;
    for(vector<char>::iterator it = pushData.data.begin(); it != pushData.data.end(); it++) {
      jsonEnd++;
      if((*it) == 0) {
        break;
      }
    }
    
    string json(&(pushData.data)[0], jsonEnd);
    
    cout << "JSON string: " << json << endl;
    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    
    bool parseSuccess = jsonReader.parse(json, jsonRoot);
    
    if(!parseSuccess) {
      cout << "JSON parsing error:" << endl;
      cout << jsonReader.getFormatedErrorMessages() << endl;
      return;
    }
    
    cout << "Parsed JSON: " << jsonRoot.toStyledString() << endl;


    ofstream spot_file;
    spot_file.open("spotreport.log", ios::in|ios::app);
    if(spot_file.is_open())
    {
      std::cout << "Logging to file [" << json << "]" << std::endl;
      spot_file << json << std::endl;
      spot_file.close();
    } else {
      std::cout << "FAILED Logging to file" << std::endl;
    }

    SpotReport sr;

    sr.content_guid = jsonRoot["contentGuid"].asString();
    sr.report_time = (long) (jsonRoot["reportTime"].asDouble()/1000.0);
    sr.reporting_unit = jsonRoot["reportingUnit"].asString();
    sr.size = jsonRoot["size"].asInt();
    sr.activity = jsonRoot["activity"].asString();
    sr.location_utm = jsonRoot["locationUtm"].asString();
    sr.enemy_unit = jsonRoot["enemyUnit"].asString();
    sr.observation_time = (long) (jsonRoot["observationTime"].asDouble()/1000.0);
    sr.unit = jsonRoot["unit"].asString();
    sr.equipment = jsonRoot["equipment"].asString();
    sr.assessment = jsonRoot["assessment"].asString();
    sr.narrative = jsonRoot["narrative"].asString();
    sr.authentication = jsonRoot["authentication"].asString();

    // make the pass data structure
    // convert MGRS to Lat/Lon

    double cLat, cLon;
    Convert_MGRS_To_Geodetic((char *)sr.location_utm.c_str(), &cLat, &cLon);
    cLat *= 180.00 / M_PI;
    cLon *= 180.00 / M_PI;
     std::cout << " Convert MGRS: "<< sr.location_utm << " to " << cLat << ", " << cLon << std::endl; 


     std::string bsomimeType ="application/vnd.edu.vu.isis.ammo.battlespace.gcm";
     
     spot_id++; 

     float speed, bearing;
     speed = 0; bearing = 0;
     std::string functionID = "U----";
     if(sr.equipment == "Dismount")
       functionID = "UCI---";
     if(sr.equipment == "Light Truck")
       functionID = "EVUL--";
     if(sr.equipment == "Mortar")
       functionID = "UCFM--";
     if(sr.equipment == "MG Squad")
       functionID = "EWRL--";
     if(sr.equipment == "Tank")
       functionID = "EVAT--";
     if(sr.equipment == "Inf Carrier")
       functionID = "EVAI--";
     if(sr.equipment == "IED")
       functionID = "EXI---";
     if(sr.equipment == "Barrier")
       functionID = "E-----";

     std::string front = "SUGP";
     if(sr.assessment == "Enemy")
       front = "SHGP";
     if(sr.assessment == "Neutral")
       front = "SNGP";
     if(sr.assessment == "Friend")
       front = "SFGP";
     if(sr.assessment == "Other")
       front = "SUGP";

     std::string modif = "-----";
     if(sr.size == 0)
       modif = "--***";
     if(sr.size == 1)
       modif = "-B***";
     if(sr.size == 2)
       modif = "-D***";     
     if(sr.size == 3)
       modif = "-E***";
     if(sr.size == 4)
       modif = "-F***";
     if(sr.size == 5)
       modif = "-H***";
     if(sr.size == 6)
       modif = "-I***";

     std::string symbolCode = front+functionID+modif;
     std::string bsodata = "test";
     //build the JSON string.
     //TODO: pass through created date from the PASS message; decompose the symbol code?
     ostringstream jsonBso;
     jsonBso << "{";
     jsonBso << "\"uuid\": \"" << sr.content_guid << "\", ";
     jsonBso << "\"longitude\": " << cLon << ", ";
     jsonBso << "\"latitude\": " <<  cLat << ", ";
     jsonBso << "\"sidc\": \"" << symbolCode << "\",";
     jsonBso << "\"speed\": " << speed << ", ";
     jsonBso << "\"heading\": " << bearing << "";
     jsonBso << "}";
     cout << jsonBso.str() << endl << flush;
     
     ostringstream bsouri;
     bsouri << "bft:bso/" << sr.content_guid;
     if(sender) {
       sender->pushData(bsouri.str(), BSO_MIME_TYPE, jsonBso.str());
     }

    // "uuid"  "sidc"; "title" "description";"longitude" "latitude"; "speed""heading";"created_date" "modified_date" "function"; "gcm_type" "standard_id" "exercise" "battle_dimension" "reality"  "echelon" "_disp"
    // "application/vnd.edu.vu.isis.ammo.battlespace.equipment""

} 


/*
 * Pull Request
 * Query containing search parameters. Need to determine functionality.
 * Aniticipated: this will return the data from the search time to present
 * Search time will be the newest record on the pull client's content provider
 */

void SpotPushReceiver::onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery) {

  string response = "asdf";
  

  ifstream spot_file;
  std::string json;
  std::vector<char> charvec;

  spot_file.open("spotreport.log", ios::in);
  while(spot_file.good() && !spot_file.eof())
  {
    spot_file >> json;
    std::cout << "Reading file [" << json << "]" << std::endl;
    // filter data here here
    std::vector<char> charv(json.begin(),json.end());

    sender->pullResponse(requestUid, pluginId, mimeType, "ammo-demo:test-object", charv);
  }

  spot_file.close();
}


