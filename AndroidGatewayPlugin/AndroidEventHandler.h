/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

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
  
  bool checkTimeout();
  
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
