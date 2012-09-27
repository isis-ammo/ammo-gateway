#ifndef SERIAL_MESSAGE_PROCESSOR_H
#define SERIAL_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "ace/UUID.h"
#include "protocol/AmmoMessages.pb.h"
#include "GatewayConnector.h"

#include <map>
#include <stdint.h>

class SerialServiceHandler;
class GatewayReceiver;

class SerialMessageProcessor : public ACE_Task <ACE_MT_SYNCH>, public ammo::gateway::GatewayConnectorDelegate, public ammo::gateway::DataPushReceiverListener, public ammo::gateway::PullResponseReceiverListener {
public:
  SerialMessageProcessor(SerialServiceHandler *serviceHandler, GatewayReceiver *receiver);
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
  
  ammo::gateway::GatewayConnector *gatewayConnector;

private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  SerialServiceHandler *commsHandler;
  
  GatewayReceiver *receiver;

  std::string deviceId;
  bool deviceIdAuthenticated;
  
  typedef std::map<std::string, uint32_t> TimestampMap;
  TimestampMap latestPliTimestamps;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
  std::string parseTerseData(int mt, const char *data, size_t terseLength, std::string &originUser );
  void parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime);
  std::string extractString(const char *terse, size_t& cursor, size_t length);
  std::string extractOldStyleString(const char *terse, size_t& cursor, size_t length);
  std::string extractBlob(const char *terse, size_t& cursor, size_t length);
  int8_t extractInt8(const char *terse, size_t& cursor, size_t length);
  int16_t extractInt16(const char *terse, size_t& cursor, size_t length);
  int32_t extractInt32(const char *terse, size_t& cursor, size_t length);
  int64_t extractInt64(const char *terse, size_t& cursor, size_t length);
  
  std::string generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created, int8_t hopCount);
  
  friend void testParseTerse();
};

#endif
