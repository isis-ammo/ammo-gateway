#ifndef GATEWAY_EVENT_HANDLER_H
#define GATEWAY_EVENT_HANDLER_H

#include <string>
#include <set>
#include <queue>

#include <ace/Thread_Mutex.h>

#include "NetworkEventHandler.h"
#include "protocol/AmmoMessages.pb.h"
#include "Enumerations.h"

class AndroidMessageProcessor;

struct QueuedMessage {
  char priority;
  unsigned long long messageCount; //messages of the same priority should come out in first-in first-out order (STL queue doesn't guarantee this)
  ammo::protocol::MessageWrapper *message;
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

class AndroidEventHandler : public ammo::gateway::internal::NetworkEventHandler<ammo::protocol::MessageWrapper, ammo::gateway::internal::SYNC_MULTITHREADED, 0xfeedbeef> {
public:
  AndroidEventHandler();
  virtual ~AndroidEventHandler();
  
  virtual void onConnect(std::string &peerAddress);
  virtual void onDisconnect();
  virtual int onMessageAvailable(ammo::protocol::MessageWrapper *msg);
  virtual int onError(const char errorCode);
  
  void send(ammo::protocol::MessageWrapper *msg);
  
  ammo::protocol::MessageWrapper *getNextReceivedMessage();
  void addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority);
  
private:
  std::string deviceId; //not authenticated; just for pretty logging
  std::string peerAddress;
  AndroidMessageProcessor *messageProcessor;
  
  ACE_Thread_Mutex receiveQueueMutex;
  
  typedef std::priority_queue<QueuedMessage, std::vector<QueuedMessage>, QueuedMessageComparison> MessageQueue;
  MessageQueue receiveQueue;
  
  unsigned long long receivedMessageCount;
  
  time_t latestMessageTime;
  unsigned int heartbeatTimeoutTime;
};

#endif
