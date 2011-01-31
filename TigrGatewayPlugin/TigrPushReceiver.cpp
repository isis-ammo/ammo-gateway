#include "TigrPushReceiver.h"

#include "TigrConfigurationManager.h"

#include "soap/soapContentServiceSoapBindingProxy.h"

#include <openssl/evp.h>

#include "log.h"

#include "wsseapi.h"
#include "wsaapi.h"

#include "json/json.h"

#include "mgrs.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

using namespace std;

const string USERNAME = "jwilliams";
const string PASSWORD = "jwilliams";
const string TIGR_INSTANCE = "http://tigr/r4/";

#define TIGR_CONTENTSERVICE_URL "http://tigr/r4/WebServices/ContentService2.asmx"

static char errorBuffer[CURL_ERROR_SIZE]; //libcurl variable for error string

TigrPushReceiver::TigrPushReceiver() {
 TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
 string tigrAddress = config->getTigrBaseAddress() + "/WebServices/ContentService2.asmx";
 
 tigrContentServiceAddress = new char[tigrAddress.length() + 1];
 
 strncpy(tigrContentServiceAddress, tigrAddress.c_str(), tigrAddress.length() + 1);
}

void TigrPushReceiver::onConnect(GatewayConnector *sender) {
  
}

void TigrPushReceiver::onDisconnect(GatewayConnector *sender) {
  
}


void TigrPushReceiver::onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser) {
  LOG_DEBUG("Got data.");
  LOG_DEBUG("  URI: " << uri);
  LOG_DEBUG("  Mime type: " << mimeType);
  
  if(mimeType == "application/vnd.edu.vu.isis.ammo.collector.event") {
    LOG_DEBUG("Extracting JSON metadata...");
    
    unsigned int jsonEnd = 0;
    for(vector<char>::iterator it = data.begin(); it != data.end(); it++) {
      jsonEnd++;
      if((*it) == 0) {
        break;
      }
    }
    
    string json(&data[0], jsonEnd);
    
    LOG_DEBUG("JSON string: " << json);
    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    
    bool parseSuccess = jsonReader.parse(json, jsonRoot);
    
    if(!parseSuccess) {
      LOG_ERROR("JSON parsing error:");
      LOG_ERROR(jsonReader.getFormatedErrorMessages());
      return;
    }
    
    LOG_DEBUG("Parsed JSON: " << jsonRoot.toStyledString());
    EventReport newEvent;
    newEvent.uri = uri;
    newEvent.title = jsonRoot["title"].asString();
    newEvent.description =   jsonRoot["description"].asString();
    newEvent.latitude = jsonRoot["latitude"].asDouble();
    newEvent.longitude = jsonRoot["longitude"].asDouble();  
    newEvent.timeSeconds = (int) (jsonRoot["createdDate"].asDouble() / 1000);
    newEvent.mediaCount = jsonRoot["mediaCount"].asInt();
    if(jsonRoot["categoryId"].isString()) {
      newEvent.categoryId = jsonRoot["categoryId"].asString();
    } else {
      newEvent.categoryId = "";
    }
    newEvent.username = originUser;
    
    int lastSlashPosition = uri.find_last_of("/");
    string eventNumberString = uri.substr(lastSlashPosition + 1);
    LOG_DEBUG("Event Number: " << eventNumberString);
    newEvent.eventNumber = atoi(eventNumberString.c_str());
    
    //add newEvent to the map
    
    
    
    if(newEvent.mediaCount == 0) {
      sendEventReport(newEvent);
    } else {
      unsentEventReports[newEvent.eventNumber] = newEvent;
    }
  } else if(mimeType == "application/vnd.edu.vu.isis.ammo.collector.media") {
    LOG_DEBUG("Extracting JSON metadata...");
    
    unsigned int jsonEnd = 0;
    bool jsonEndFound = false;
    unsigned int tagEnd = 0;
    for(vector<char>::iterator it = data.begin(); it != data.end(); it++) {
      if(!jsonEndFound) {
        jsonEnd++;
      }
      tagEnd++;
      if(*it == 0) {
        if(!jsonEndFound) {
          jsonEndFound = true;
        } else {
          break;
        }
      }
    }
    
    
    
    string json(&data[0], jsonEnd);
    
    LOG_DEBUG("JSON string: " << json);
    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    
    bool parseSuccess = jsonReader.parse(json, jsonRoot);
    
    if(!parseSuccess) {
      LOG_ERROR("JSON parsing error:");
      LOG_ERROR(jsonReader.getFormatedErrorMessages());
      return;
    }
    
    LOG_DEBUG("Parsed JSON: " << jsonRoot.toStyledString());
    MediaObject newMedia;
    newMedia.uri = uri;
    string filePath = jsonRoot["data"].asString();
    int lastSlashPosition = filePath.find_last_of("/");
    newMedia.filename = filePath.substr(lastSlashPosition + 1);
    newMedia.mimeType = jsonRoot["dataType"].asString();
    newMedia.associatedEventId = jsonRoot["eventId"].asInt();
    
    
    
    unsigned int dataSize = *((int *) &data[tagEnd]);
    
    LOG_DEBUG("Media Data Size:" << dataSize);
    
    unsigned int maxSize = data.size() - tagEnd - 8;
    LOG_DEBUG("Maximum valid data size: " << maxSize);
    
    if(dataSize > maxSize) {
      LOG_ERROR("Error:  data size is larger than available data.");
      return;
    }
    
    
    LOG_INFO("Uploading image to TIGR with name " << newMedia.filename << "...");
    string fileReference = uploadMedia(&data[tagEnd + 4], dataSize, newMedia.filename, newMedia.mimeType, originUser);
    if(fileReference != "") {
      string imageCid = createMedia(newMedia.filename, fileReference, originUser);
      unsentEventReports[newMedia.associatedEventId].associatedMediaCids.push_back(imageCid);
      if(unsentEventReports[newMedia.associatedEventId].associatedMediaCids.size() == unsentEventReports[newMedia.associatedEventId].mediaCount) {
        sendEventReport(unsentEventReports[newMedia.associatedEventId]);
        unsentEventReports.erase(newMedia.associatedEventId);
      }
    }
  } else if(mimeType == "application/vnd.edu.vu.isis.ammo.report.report_base") {
    LOG_DEBUG("Extracting JSON metadata...");
    
    unsigned int jsonEnd = 0;
    for(vector<char>::iterator it = data.begin(); it != data.end(); it++) {
      jsonEnd++;
      if((*it) == 0) {
        break;
      }
    }
    
    string json(&data[0], jsonEnd);
    
    LOG_DEBUG("JSON string: " << json);
    
    Json::Value jsonRoot;
    Json::Reader jsonReader;
    
    bool parseSuccess = jsonReader.parse(json, jsonRoot);
    
    if(!parseSuccess) {
      LOG_ERROR("JSON parsing error:");
      LOG_ERROR(jsonReader.getFormatedErrorMessages());
      return;
    }
    
    LOG_DEBUG("Parsed JSON: " << jsonRoot.toStyledString());

    SpotReport sr;
    sr.content_guid = jsonRoot["contentGuid"].asString();
    sr.report_time = (long) (jsonRoot["reportTime"].asDouble()/1000.0);
    sr.reporting_unit = jsonRoot["reportingUnit"].asString();
//    sr.size = jsonRoot["size"].asInt();
    sr.activity = jsonRoot["activity"].asString();
    sr.location_utm = jsonRoot["locationUtm"].asString();
    sr.enemy_unit = jsonRoot["enemyUnit"].asString();
    sr.observation_time = (long) (jsonRoot["observationTime"].asDouble()/1000.0);
    sr.unit = jsonRoot["unit"].asString();
    sr.equipment = jsonRoot["equipment"].asString();
    sr.assessment = jsonRoot["assessment"].asString();
    sr.narrative = jsonRoot["narrative"].asString();
    sr.authentication = jsonRoot["authentication"].asString();

    LOG_DEBUG("Done reading spotreport");


    EventReport newEvent;
    newEvent.uri = uri;
    newEvent.title =  "SPOT: "+sr.reporting_unit; //jsonRoot["title"].asString();
    std::string des = "SPOT REPORT [SPOTREP]\n";
    des += "DATE AND TIME:  ";
    unsigned long long timeMilliseconds = sr.report_time;
    time_t rawtime;
    rawtime = sr.report_time;
    des += ctime(&rawtime);
    LOG_DEBUG("raw: " << rawtime << "  ctime: " << des);
    des += "UNIT:           " + sr.reporting_unit + "\n";
    //    des += "SIZE:           " + size_strings[sr.size] + "\n";
    des += "ACTIVITY:       " + sr.activity+"\n";
    des += "LOCATION:       " + sr.location_utm+"\n";
    des += "ENEMY:          " + sr.enemy_unit+"\n";
    //des += "TIME:           " + sr.observation_time+"\n";
    des += "EQUIPMENT:      " + sr.equipment+"\n";
    des += "ASSESSMENT:     " + sr.assessment+"\n";
    des += "NARRATIVE:      " + sr.narrative+"\n";
    des += "AUTHENTICATION: " + sr.authentication+"\n";

    newEvent.description = des;

    // convert MGRS to Lat/Lon

    double cLat, cLon;
      char cMgrs[45];
      long convStatus;
    Convert_MGRS_To_Geodetic((char *)sr.location_utm.c_str(), &cLat, &cLon);
    cLat *= 180.00 / M_PI;
    cLon *= 180.00 / M_PI;
     LOG_DEBUG(" Convert MGRS: "<< sr.location_utm << " to " << cLat << ", " << cLon); 


    newEvent.latitude = cLat; //jsonRoot["latitude"].asDouble();
    newEvent.longitude = cLon; //jsonRoot["longitude"].asDouble();  
    newEvent.timeSeconds = (time_t) sr.report_time ; //(jsonRoot["createdDate"].asDouble() / 1000);
    newEvent.mediaCount = 0; //jsonRoot["mediaCount"].asInt();
    newEvent.categoryId = "01010203020000000000000000000000"; //jsonRoot["categoryId"].asString();//
    newEvent.username = originUser;
    
    int lastSlashPosition = uri.find_last_of("/");
    string eventNumberString = uri.substr(lastSlashPosition + 1);
    LOG_DEBUG("Event Number: " << eventNumberString << "At:(" << cLat << "," << cLon << ")  Descr[" << des);
    newEvent.eventNumber = atoi(eventNumberString.c_str());
    
    //add newEvent to the map
    
    if(newEvent.mediaCount == 0) {
      sendEventReport(newEvent);
    } else {
      unsentEventReports[newEvent.eventNumber] = newEvent;
    }  
  }

  else {
    LOG_ERROR("ERROR!  Invalid Mime Type.");
  }
  
}

void TigrPushReceiver::onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery) {

  string response = "asdf";
  
  vector<string> jsonResults;
  
  get(query, jsonResults);
  
  for(vector<string>::iterator it = jsonResults.begin(); it != jsonResults.end(); it++) {
    
    vector<char> data(it->begin(), it->end());
    
    sender->pullResponse(requestUid, pluginId, mimeType, "ammo-demo:test-object", data);
  }
}

bool TigrPushReceiver::get(std::string query, std::vector<std::string> &jsonResults) {
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  ContentServiceSoapBindingProxy proxy;
  
  soap_register_plugin(&proxy, soap_wsse);
  soap_register_plugin(&proxy, soap_wsa);
  
  ostringstream endpointUrl;
  
  proxy.soap_endpoint = tigrContentServiceAddress;
  
  ns2__GetType getBody;
  
  getBody.queryExpression = query;
  getBody.maxResults = "100";
  getBody.startFrom = "0";
  
  soap_wsse_add_Security(&proxy);
  
  pair<string, string> credentials = config->getTigrCredentialsForUser(""); //blank username gives the default
  
  soap_wsse_add_UsernameTokenText(&proxy, NULL, credentials.first.c_str(), credentials.second.c_str());
  
  //soap_wsa_request(&proxy, NULL, NULL, "http://www.ascendintel.com/tigr/v2/cs#Create");
  
  ns2__GetResponseType response;
  
  LOG_INFO("Sending get request (for media)...");
  if(proxy.GetOperation(&getBody, &response) == SOAP_OK) {
    LOG_INFO("done.");
    LOG_TRACE("Response: __size_GetResponseType = " << response.__size_GetResponseType);
    for(int i=0; i<response.__size_GetResponseType; i++) {
      string jsonString = jsonForObject(response.__union_GetResponseType[i]);
      if(jsonString != "") {
        jsonResults.push_back(jsonString);
      }
    }
    return true;
  } else {
    proxy.soap_stream_fault(std::cerr);
    return false;
  }
  
}

string TigrPushReceiver::jsonForObject(__ns2__union_GetResponseType &obj) {
  if(obj.__union_GetResponseType == SOAP_UNION__ns2__union_GetResponseType_Event) {
    string cid = obj.union_GetResponseType.Event->cid;
    string title = obj.union_GetResponseType.Event->title;
    string description = obj.union_GetResponseType.Event->description ? *(obj.union_GetResponseType.Event->description) : "(not set)";
    string category = "(FIXME: need to extract category)";
    string unit = "(FIXME: need to extract unit)";
    double latitude = 0, longitude = 0;
    if(obj.union_GetResponseType.Event->locationList) {
      if(obj.union_GetResponseType.Event->locationList->__sizeAbstractGeometry == 1) {
        if(obj.union_GetResponseType.Event->locationList->__union_LocationListType->__unionAbstractGeometry == SOAP_UNION__ns2__union_LocationListType_ns3__Point) {
          string coords = obj.union_GetResponseType.Event->locationList->__union_LocationListType->__union_LocationListType.ns3__Point->coordinates;
          //LOG_INFO("Coords: " << coords);
          //find the space separator
          int sep = coords.find(" ");
          if(sep != string::npos) {
            string latString = coords.substr(0, sep);
            string longString = coords.substr(sep);
            latitude = atof(latString.c_str());
            longitude = atof(longString.c_str());
          } else {
            LOG_ERROR("Invalid coordinate");
          }
          
        } else {
          LOG_ERROR("Location error: Only Points are supported at this time.");
        }
      } else {
        LOG_ERROR("Location error: More than one location not supported at this time.");
      }
    }
    
    LOG_TRACE("Got an event:");
    LOG_TRACE("  cid: " << cid);
    LOG_TRACE("  Title: " << title);
    LOG_TRACE("  Description: " << description);
    LOG_TRACE("  Category: " << category);
    LOG_TRACE("  Unit: " << unit);
    LOG_TRACE("  Position: " << latitude << ", " << longitude);
    
    Json::Value root;
    
    root["cid"] = cid;
    root["title"] = title;
    root["description"] = description;
    root["category"] = category;
    root["unit"] = unit;
    root["latitude"] = latitude;
    root["longitude"] = longitude;
    
    LOG_DEBUG("JSON: " << root.toStyledString());
    return root.toStyledString();
  } else {
    LOG_ERROR("Not an event...  only events are supported at this time.");
  }
  return "";
}

string TigrPushReceiver::createMedia(string filename, string fileReference, string username) {
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  ContentServiceSoapBindingProxy proxy;
  
  soap_register_plugin(&proxy, soap_wsse);
  soap_register_plugin(&proxy, soap_wsa);
  
  ostringstream endpointUrl;
  
  proxy.soap_endpoint = tigrContentServiceAddress;
  
  ns2__CreateType createBody;
  
  createBody.__unionAbstractContentUserProperties = SOAP_UNION__ns2__union_CreateType_MediaUserProperties;
  createBody.__union_CreateType.MediaUserProperties = new ns2__MediaUserPropertiesType();
  
  createBody.__union_CreateType.MediaUserProperties->securityInfo = config->getTigrSecurityInfo();
  createBody.__union_CreateType.MediaUserProperties->title = filename;
  createBody.__union_CreateType.MediaUserProperties->fileReference = new string(fileReference);
  
  soap_wsse_add_Security(&proxy);
  
  pair<string, string> credentials = config->getTigrCredentialsForUser(username);
  
  soap_wsse_add_UsernameTokenText(&proxy, NULL, credentials.first.c_str(), credentials.second.c_str());
  
  //soap_wsa_request(&proxy, NULL, NULL, "http://www.ascendintel.com/tigr/v2/cs#Create");
  
  ns2__CreateResponseType response;
  
  LOG_INFO("Sending create request (for media)...");
  if(proxy.CreateOperation(&createBody, &response) == SOAP_OK) {
    LOG_INFO("done.");
    LOG_DEBUG("Response: new cid is " << response.cid);
    return response.cid;
  } else {
    proxy.soap_stream_fault(std::cerr);
    return "";
  }
}

bool TigrPushReceiver::sendEventReport(EventReport &report) {
  createEvent(report.title, report.description, report.latitude, report.longitude, report.timeSeconds, report.categoryId, report.associatedMediaCids, report.username);
}

bool TigrPushReceiver::createEvent(string title, string description, double latitude, double longitude, time_t timeSeconds, string categoryId, vector<string> mediaCids, string username) {
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  ostringstream latLongStringStream;
  latLongStringStream << latitude << " " << longitude;
  string latLongString = latLongStringStream.str();
  
  ContentServiceSoapBindingProxy proxy;
  
  soap_register_plugin(&proxy, soap_wsse);
  soap_register_plugin(&proxy, soap_wsa);
  
  proxy.soap_endpoint = tigrContentServiceAddress;
  
  ns2__CreateType createBody;
  
  createBody.__unionAbstractContentUserProperties = SOAP_UNION__ns2__union_CreateType_EventUserProperties;
  createBody.__union_CreateType.EventUserProperties = new ns2__EventUserPropertiesType();
  
  createBody.__union_CreateType.EventUserProperties->securityInfo = config->getTigrSecurityInfo();
  createBody.__union_CreateType.EventUserProperties->title = title;
  createBody.__union_CreateType.EventUserProperties->description = new string(description);
  
  ns2__CategoryListType *categoryList = new ns2__CategoryListType();
  if(categoryId != "") {
    categoryList->category.push_back(categoryId);
  } else {
    categoryList->category.push_back("01010203020000000000000000000000");
  }
  createBody.__union_CreateType.EventUserProperties->categoryList = categoryList;
  
  ns2__LocationListType *locationList = new ns2__LocationListType();
  locationList->__sizeAbstractGeometry = 1;
  locationList->__union_LocationListType = new __ns2__union_LocationListType[1];
  locationList->__union_LocationListType[0].__unionAbstractGeometry = SOAP_UNION__ns2__union_LocationListType_ns3__Point;
  locationList->__union_LocationListType[0].__union_LocationListType.ns3__Point = new ns3__PointType();
  locationList->__union_LocationListType[0].__union_LocationListType.ns3__Point->coordinates = latLongString;
  createBody.__union_CreateType.EventUserProperties->locationList = locationList;
  
  createBody.__union_CreateType.EventUserProperties->ns3__timePeriod = new ns3__TimePeriodType();
  createBody.__union_CreateType.EventUserProperties->ns3__timePeriod->start = timeSeconds;
  createBody.__union_CreateType.EventUserProperties->ns3__timePeriod->end = timeSeconds + 1;
  
  if(mediaCids.size() != 0) {
    ns2__AbstractContentReferenceListType *refList = new ns2__AbstractContentReferenceListType();
    refList->__sizeAbstractContentReference = mediaCids.size();
    refList->__union_AbstractContentReferenceListType = new __ns2__union_AbstractContentReferenceListType[mediaCids.size()];
    for(int i = 0; i < mediaCids.size(); i++) {
      refList->__union_AbstractContentReferenceListType[i].__unionAbstractContentReference = SOAP_UNION__ns2__union_AbstractContentReferenceListType_ContentIdentifierReference;
      refList->__union_AbstractContentReferenceListType[i].__union_AbstractContentReferenceListType.ContentIdentifierReference = new ns2__ContentIdentifierReferenceType();
      refList->__union_AbstractContentReferenceListType[i].__union_AbstractContentReferenceListType.ContentIdentifierReference->cid = mediaCids[i];
    }
    createBody.__union_CreateType.EventUserProperties->children = refList;
  }
  
  soap_wsse_add_Security(&proxy);
  
  pair<string, string> credentials = config->getTigrCredentialsForUser(username);
  
  soap_wsse_add_UsernameTokenText(&proxy, NULL, credentials.first.c_str(), credentials.second.c_str());
  
  //soap_wsa_request(&proxy, NULL, NULL, "http://www.ascendintel.com/tigr/v2/cs#Create");
  
  ns2__CreateResponseType response;
  
  LOG_INFO("Sending create request (for event)...");
  if(proxy.CreateOperation(&createBody, &response) == SOAP_OK) {
    LOG_INFO("done.");
    LOG_DEBUG("Response: new cid is " << response.cid);
    return true;
  } else {
    proxy.soap_stream_fault(std::cerr);
    return false;
  }
}

string TigrPushReceiver::uploadMedia(char *data, long length, string filename, string mimeType, std::string username) {
  TigrConfigurationManager *config = TigrConfigurationManager::getInstance();
  
  pair<string, string> credentials = config->getTigrCredentialsForUser(username);
  
  CURL *curl;
  CURLcode res;
  
  res = curl_global_init(CURL_GLOBAL_ALL);
  
  struct curl_httppost *formpost=NULL;
  struct curl_httppost *lastptr=NULL;
  
  curl_formadd(&formpost, &lastptr,
               CURLFORM_COPYNAME, "username",
               CURLFORM_COPYCONTENTS, credentials.first.c_str(),
               CURLFORM_END);
  
  curl_formadd(&formpost, &lastptr,
               CURLFORM_COPYNAME, "password",
               CURLFORM_COPYCONTENTS, credentials.second.c_str(),
               CURLFORM_END);
  
  curl_formadd(&formpost, &lastptr,
               CURLFORM_COPYNAME, "file0",
               CURLFORM_BUFFER, filename.c_str(),
               CURLFORM_BUFFERPTR, data,
               CURLFORM_BUFFERLENGTH, length,
               CURLFORM_CONTENTTYPE, mimeType.c_str(),
               CURLFORM_END);
  
  curl = curl_easy_init();
  if(curl) {
    res = curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to set error buffer.");
      return "";
    }
    
    res = curl_easy_setopt(curl, CURLOPT_URL, (config->getTigrBaseAddress() + "/WebServices/MediaFilePostHandler.ashx").c_str());
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to set URL: " << errorBuffer);
      return "";
    }
    
    res = curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to set post data: " << errorBuffer);
      return "";
    }
    
    string returnedData = "";
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to set writer: " << errorBuffer);
      return "";
    }
    
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnedData);
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to set write data: " << errorBuffer);
      return "";
    }
    
    res = curl_easy_perform(curl);
    
    if(res != CURLE_OK) {
      LOG_ERROR("Failed to POST data: " << errorBuffer);
      returnedData = "";
    }
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
    
    LOG_DEBUG("Returned data from POST: " << returnedData);
    return returnedData;
  }
}

static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData) {
  if(writerData == NULL) {
    return 0;
  }
  
  writerData->append(data, size*nmemb);
  
  return size * nmemb;
}
