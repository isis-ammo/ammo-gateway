#include "GatewayConnector.h"

#include "SpotPushReceiver.h"

#include "json/json.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
//#include <boost/tokenizer.hpp>
//#include <boost/foreach.hpp>
//#include <boost/algorithm/string.hpp>


using namespace std;

void SpotPushReceiver::onConnect(GatewayConnector *sender) {
  
}

void SpotPushReceiver::onDisconnect(GatewayConnector *sender) {
  
}

void SpotPushReceiver::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser) {
  cout << "Got data." << endl;
  cout << "  URI: " << uri << endl;
  cout << "  Mime type: " << mimeType << endl;
  cout << "  Data: " << string(data.begin(), data.end()) << endl;
  cout << "  Origin Username: " << originUser << endl;

    cout << "Extracting JSON metadata..." << endl << flush;
    
    unsigned int jsonEnd = 0;
    for(vector<char>::iterator it = data.begin(); it != data.end(); it++) {
      jsonEnd++;
      if((*it) == 0) {
        break;
      }
    }
    
    string json(&data[0], jsonEnd);
    
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

    SpotReport sr;
    sr.content_guid = jsonRoot["content_guid"].asString();
    sr.report_time = atol(jsonRoot["report_time"].asString().c_str());
    sr.reporting_unit = jsonRoot["reporting_unit"].asString();
    sr.size = jsonRoot["size"].asString();
    sr.activity = jsonRoot["activity"].asString();
    sr.location_utm = jsonRoot["location_utm"].asString();
    sr.enemy_unit = jsonRoot["enemy_unit"].asString();
    sr.observation_time = atol(jsonRoot["observation_time"].asString().c_str());
    sr.unit = jsonRoot["unit"].asString();
    sr.equipment = jsonRoot["equipment"].asString();
    sr.assessment = jsonRoot["assessment"].asString();
    sr.narrative = jsonRoot["narrative"].asString();
    sr.authenitication = jsonRoot["authenitication"].asString();

    // make the pass data structure
    
    //sender->pushData(std::string uri,std::string mimeType,const std::string &data);
    std::string uri =
      std::string mimeType ="application/vnd.edu.vu.isis.ammo.battlespace.gcm";

      // "application/vnd.edu.vu.isis.ammo.battlespace.gcm_anchor";

    sender->pushData( uri,std::string mimeType,const std::string &data);
 "uuid"  "sidc"; "title" "description";"longitude" "latitude"; "speed""heading";"created_date" "modified_date" "function"; "gcm_type" "standard_id" "exercise" "battle_dimension" "reality"  "echelon" "_disp"

															     // "application/vnd.edu.vu.isis.ammo.battlespace.equipment""

} 
