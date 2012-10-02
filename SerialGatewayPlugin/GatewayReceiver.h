/*
 * GatewayReceiver.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef GATEWAYRECEIVER_H_
#define GATEWAYRECEIVER_H_

#include "GatewayConnector.h"
#include <stdint.h>
#include <map>

class GpsThread;

struct PliInfo {
  int32_t lat;
  int32_t lon;
  int32_t createdTime;
};

class GatewayReceiver : public ammo::gateway::DataPushReceiverListener {
public:
  GatewayReceiver(GpsThread *gpsThread);
  virtual ~GatewayReceiver();

  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  std::string *getNextReceivedMessage();
  void addReceivedMessage(std::string *msg);

  int getNextMessageSize();
  bool isMessageAvailable();

  std::string getNextPliRelayPacket();

  void appendString(std::ostringstream &stream, const std::string &str);

  void appendInt64(std::ostringstream &stream, const int64_t val);

  void appendInt32(std::ostringstream &stream, const int32_t val);
  void appendUInt32(std::ostringstream &stream, const uint32_t val);

  void appendInt16(std::ostringstream &stream, const int16_t val);
  void appendUInt16(std::ostringstream &stream, const uint16_t val);

  void appendInt8(std::ostringstream &stream, const int8_t val);

  void appendBlob(std::ostringstream &stream, const std::string &blob);

  void addPli(std::string &username, int32_t lat, int32_t lon, int32_t createdTime);

private:
  GpsThread *gpsThread;
  
  ACE_Thread_Mutex receiveQueueMutex;

  typedef std::queue<std::string *> MessageQueue;
  MessageQueue receiveQueue;

  unsigned long long receivedMessageCount;

  typedef std::map<std::string, PliInfo> PliMap;
  PliMap pliMap;
  ACE_Thread_Mutex pliMapMutex;

  bool pliRelayEnabled;
  int pliRelayPerCycle;
  int pliIndex;
  std::string pliRelayNodeName;
  
  int rangeScale;
  int timeScale;
};

#endif /* GATEWAYRECEIVER_H_ */
