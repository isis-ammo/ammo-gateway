#ifndef LOCATION_STORE_H
#define LOCATION_STORE_H

#include "ace/streams.h"

#include "GatewayConnector.h"

class LocationStoreReceiver : public DataPushReceiverListener,
							  public GatewayConnectorDelegate
{
public:
  LocationStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (GatewayConnector *sender);
  virtual void onDisconnect (GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onDataReceived (GatewayConnector *sender,
							   std::string uri,
							   std::string mimeType,
							   std::vector<char> &data);
	
private:
  std::string filename_;
  std::ofstream out_;
};

#endif        //  #ifndef LOCATION_STORE_H

