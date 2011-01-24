#ifndef SPOT_PUSH_RECEIVER_H
#define SPOT_PUSH_RECEIVER_H

#include "GatewayConnector.h"



class SpotPushReceiver : public DataPushReceiverListener, public GatewayConnectorDelegate {
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser);
};

class SpotReport {
public:
  std::string content_guid;
  long report_time;
  std::string reporting_unit;
  std::string size;
  std::string activity;
  std::string location_utm;
  std::string enemy_unit;
  long observation_time;
  std::string unit;
  std::string equipment;
  std::string assessment;
  std::string narrative;
  std::string authenitication;

};


#endif        //  #ifndef SPOT_PUSH_RECEIVER_H

