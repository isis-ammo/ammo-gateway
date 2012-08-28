/*
 * GatewayReceiver.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef GATEWAYRECEIVER_H_
#define GATEWAYRECEIVER_H_

#include "GatewayConnector.h"

struct QueuedMessage {
  char priority;
  unsigned long long messageCount; //messages of the same priority should come out in first-in first-out order (STL queue doesn't guarantee this)
  std::string *message;
};

class QueuedMessageComparison {
public:
  bool operator()(QueuedMessage &first, QueuedMessage &second) { //returns true if first is lower priority than second
    if(first.priority < second.priority) {
      return true;
    } else if(first.priority == second.priority && first.messageCount > second.messageCount) {
      return true;
    } else {
      return false;
    }
  }
};

class GatewayReceiver : public ammo::gateway::DataPushReceiverListener {
public:
  GatewayReceiver();
  virtual ~GatewayReceiver();

  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  std::string *getNextReceivedMessage();
  void addReceivedMessage(std::string *msg, char priority);

  int getNextMessageSize();

private:
  ACE_Thread_Mutex receiveQueueMutex;

  typedef std::priority_queue<QueuedMessage, std::vector<QueuedMessage>, QueuedMessageComparison> MessageQueue;
  MessageQueue receiveQueue;

  unsigned long long receivedMessageCount;
};

#endif /* GATEWAYRECEIVER_H_ */
