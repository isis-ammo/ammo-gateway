#ifndef LOCATION_STORE_H
#define LOCATION_STORE_H

#include "GatewayConnector.h"

class sqlite3;

class LocationStoreReceiver : public DataPushReceiverListener,
							  public GatewayConnectorDelegate
{
public:
  LocationStoreReceiver (void);
  ~LocationStoreReceiver (void);
	
  // GatewayConnectorDelegate methods
  virtual void onConnect (GatewayConnector *sender);
  virtual void onDisconnect (GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onDataReceived (GatewayConnector *sender,
							   std::string uri,
							   std::string mimeType,
							   std::vector<char> &data);
	
private:
  // For debugging, maps SQLITE error codes to informative strings.
  const char * const ec_to_string (int error_code) const;
	
private:
  // Pointer to open database.
  sqlite3 *db_;
	
  // Common prefix to error messages from callbacks
  // to this class.
  const char * const err_prefix_;
};

#endif        //  #ifndef LOCATION_STORE_H

