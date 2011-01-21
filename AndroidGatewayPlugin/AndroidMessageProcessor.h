#ifndef ANDROID_MESSAGE_PROCESSOR_H
#define ANDROID_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "protocol/AmmoMessages.pb.h"
#include "GatewayConnector.h"

class AndroidServiceHandler;

class AndroidMessageProcessor : public ACE_Task <ACE_MT_SYNCH>, public GatewayConnectorDelegate, public DataPushReceiverListener, public PullResponseReceiverListener {
public:
  AndroidMessageProcessor(AndroidServiceHandler *serviceHandler);
  virtual ~AndroidMessageProcessor();
  
  virtual int open(void *args);
  virtual int close(unsigned long flags);
  
  virtual int svc();
  
  void signalNewMessageAvailable();
  
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
  
private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  AndroidServiceHandler *commsHandler;
  
  GatewayConnector *gatewayConnector;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
};

#endif
