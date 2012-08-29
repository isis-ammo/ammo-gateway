/*
 * GatewayReceiver.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef GATEWAYRECEIVER_H_
#define GATEWAYRECEIVER_H_

#include "GatewayConnector.h"

class GatewayReceiver : public ammo::gateway::DataPushReceiverListener {
public:
  GatewayReceiver();
  virtual ~GatewayReceiver();

  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  std::string *getNextReceivedMessage();
  void addReceivedMessage(std::string *msg);

  int getNextMessageSize();
  bool isMessageAvailable();

  void appendString(std::ostringstream &stream, std::string &str);

  void appendInt32(std::ostringstream &stream, int32_t val);

  void appendUInt32(std::ostringstream &stream, uint32_t val);
  void appendUInt16(std::ostringstream &stream, uint16_t val);

  void appendBlob(std::ostringstream &stream, std::string &blob);

private:
  ACE_Thread_Mutex receiveQueueMutex;

  typedef std::queue<std::string *> MessageQueue;
  MessageQueue receiveQueue;

  unsigned long long receivedMessageCount;
};

#endif /* GATEWAYRECEIVER_H_ */
