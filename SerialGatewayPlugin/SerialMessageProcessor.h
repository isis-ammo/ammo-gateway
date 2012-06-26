#ifndef SERIAL_MESSAGE_PROCESSOR_H
#define SERIAL_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "ace/UUID.h"
#include "protocol/AmmoMessages.pb.h"
#include "GatewayConnector.h"

#include <map>

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
  
  typedef std::map<std::string, uint32_t> TimestampMap;
  TimestampMap latestPliTimestamps;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
  std::string parseTerseData(int mt, const char *data, std::string &originUser );
  void parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime);
  std::string extractString(const char *terse, int& cursor);
  std::string extractOldStyleString(const char *terse, int& cursor);
  std::string extractBlob(const char *terse, int& cursor);
  int8_t extractInt8(const char *terse, int& cursor);
  int16_t extractInt16(const char *terse, int& cursor);
  int32_t extractInt32(const char *terse, int& cursor);
  int64_t extractInt64(const char *terse, int& cursor);
  
  std::string generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created);
  
  friend void testParseTerse();
};

#endif
