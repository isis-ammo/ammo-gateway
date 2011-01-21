#ifndef ANDROID_SERVICE_HANDLER_H
#define ANDROID_SERVICE_HANDLER_H

#include "GatewayConnector.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/AmmoMessages.pb.h"
#include <vector>
#include <queue>

class AndroidMessageProcessor;

class AndroidServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH>, public GatewayConnectorDelegate, public DataPushReceiverListener, public PullResponseReceiverListener {
public:
  AndroidServiceHandler();
  int open(void *ptr = 0);
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum);
  
  void sendMessage(ammo::protocol::MessageWrapper *msg);
  ammo::protocol::MessageWrapper *getNextMessageToSend();
  
  ammo::protocol::MessageWrapper *getNextReceivedMessage();
  void addReceivedMessage(ammo::protocol::MessageWrapper *msg);
  
  //GatewayConnectorDelegate methods
  virtual void onConnect(GatewayConnector *sender);
  virtual void onDisconnect(GatewayConnector *sender);
  virtual void onAuthenticationResponse(GatewayConnector *sender, bool result);
  
  //DataPushReceiverListener methods
  virtual void onDataReceived(GatewayConnector *sender, std::string uri, std::string mimeType, std::vector<char> &data, std::string originUser);

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
  
  std::string deviceId; //not validated; just for pretty logging
  
  AndroidMessageProcessor *messageProcessor;
  ACE_Thread_Mutex sendQueueMutex;
  ACE_Thread_Mutex receiveQueueMutex;
  
  std::queue<ammo::protocol::MessageWrapper *> sendQueue;
  std::queue<ammo::protocol::MessageWrapper *> receiveQueue;
  
  GatewayConnector *gatewayConnector;
};

#endif        //  #ifndef ANDROID_SERVICE_HANDLER_H

