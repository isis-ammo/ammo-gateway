#ifndef TIGR_PUSH_RECEIVER_H
#define TIGR_PUSH_RECEIVER_H

#include "GatewayConnector.h"
#include <ctime>
#include <vector>

class __ns2__union_GetResponseType; //forward declaration of object containing individual TIGR objects

class MediaObject {
public:
  std::string uri;
  std::string filename;
  std::string mimeType;
  int associatedEventId;
  std::vector<char> data;
  std::string username;
};

class EventReport {
public:
  std::string uri;
  int eventNumber;
  std::string title;
  std::string description;
  std::string displayName;
  double latitude;
  double longitude;
  std::string categoryId;
  time_t timeSeconds;
  int mediaCount;
  std::vector<std::string> associatedMediaCids;
  std::string username;
};

class SpotReport {
public:
  std::string content_guid;
  long report_time;
  std::string reporting_unit;
  int size;
  std::string activity;
  std::string location_utm;
  std::string enemy_unit;
  long observation_time;
  std::string unit;
  std::string equipment;
  std::string assessment;
  std::string narrative;
  std::string authentication;

};


class TigrPushReceiver : public ammo::gateway::DataPushReceiverListener, public ammo::gateway::GatewayConnectorDelegate, public ammo::gateway::PullRequestReceiverListener {
public:
  TigrPushReceiver();
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser);
  
  bool get(std::string query, std::vector<std::string> &jsonResults);
  std::string jsonForObject(__ns2__union_GetResponseType &obj);
  
  std::string uploadMedia(char *data, long length, std::string filename, std::string mimeType, std::string username);
  
  std::string createMedia(std::string filename, std::string fileReference, std::string username);
  bool createEvent(std::string title, std::string description, double latitude, double longitude, time_t timeSeconds, std::string categoryId, std::vector<std::string> mediaCids, std::string username);
  
  bool sendEventReport(EventReport &report);
  
  //PullRequestReceiverListener methods
  virtual void onPullRequestReceived(ammo::gateway::GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery);
  
private:
  std::map<int, EventReport> unsentEventReports;
  char *tigrContentServiceAddress;
};

static int write_callback(char *data, size_t size, size_t nmemb, std::string *writerData);

#endif        //  #ifndef TIGR_PUSH_RECEIVER_H
