#ifndef ANDROID_SERVICE_HANDLER_H
#define ANDROID_SERVICE_HANDLER_H

#include "GatewayConnector.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/AmmmoMessages.pb.h"
#include <vector>

class AndroidServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>, public GatewayConnectorDelegate, public DataPushReceiverListener, public PullResponseReceiverListener {
public:
  AndroidServiceHandler();
  int open(void *ptr = 0);
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  void sendData(ammmo::protocol::MessageWrapper &msg);
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum);
  
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  virtual void onAuthenticationResponse(GatewayConnector *sender, bool result);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data);

  //PullResponseReceiverListener method
  virtual void onDataReceived(GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId, std::string mimeType,
			      std::string uri, std::vector<char> &data);


  
  ~AndroidServiceHandler();
  
protected:
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
  
  typedef enum {
    READING_SIZE = 0,
    READING_CHECKSUM = 1,
    READING_DATA = 2
  } ReaderState;
  
  ReaderState state;
  unsigned int dataSize;
  unsigned int checksum;
  char *collectedData;
  unsigned int position;
  
  GatewayConnector *gatewayConnector;
};

#endif        //  #ifndef ANDROID_SERVICE_HANDLER_H

