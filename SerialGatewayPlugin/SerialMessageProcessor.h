#ifndef SERIAL_MESSAGE_PROCESSOR_H
#define SERIAL_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "protocol/AmmoMessages.pb.h"
#include "GatewayConnector.h"

class SerialServiceHandler;

class SerialMessageProcessor : public ACE_Task <ACE_MT_SYNCH>, public ammo::gateway::GatewayConnectorDelegate, public ammo::gateway::DataPushReceiverListener, public ammo::gateway::PullResponseReceiverListener {
public:
  SerialMessageProcessor(SerialServiceHandler *serviceHandler);
  virtual ~SerialMessageProcessor();
  
  virtual int open(void *args);
  virtual int close(unsigned long flags);
  
  virtual int svc();
  
  void signalNewMessageAvailable();
  
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  virtual void onAuthenticationResponse(ammo::gateway::GatewayConnector *sender, bool result);
  
  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  //PullResponseReceiverListener method
  virtual void onPullResponseReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PullResponse &response);
  
private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  SerialServiceHandler *commsHandler;
  
  ammo::gateway::GatewayConnector *gatewayConnector;
  
  std::string deviceId;
  bool deviceIdAuthenticated;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
  std::string parseTerseData(int mt, const char *data );
  friend void testParseTerse();
};

#endif
