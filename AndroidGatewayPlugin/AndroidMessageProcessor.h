/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef ANDROID_MESSAGE_PROCESSOR_H
#define ANDROID_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "protocol/AmmoMessages.pb.h"
#include "GatewayConnector.h"

class AndroidEventHandler;

class AndroidMessageProcessor : public ACE_Task <ACE_MT_SYNCH>, public ammo::gateway::GatewayConnectorDelegate, public ammo::gateway::DataPushReceiverListener, public ammo::gateway::PullResponseReceiverListener {
public:
  AndroidMessageProcessor(AndroidEventHandler *serviceHandler);
  virtual ~AndroidMessageProcessor();
  
  virtual int open(void *args);
  virtual int close(unsigned long flags);
  
  virtual int svc();
  
  void signalNewMessageAvailable();
  
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  virtual void onAuthenticationResponse(ammo::gateway::GatewayConnector *sender, bool result);
  virtual void onPushAcknowledgementReceived(ammo::gateway::GatewayConnector *sender, const ammo::gateway::PushAcknowledgement &ack);
  
  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  //PullResponseReceiverListener method
  virtual void onPullResponseReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PullResponse &response);
  
private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  AndroidEventHandler *commsHandler;
  
  ammo::gateway::GatewayConnector *gatewayConnector;
  
  std::string deviceId;
  std::string userId;
  bool deviceIdAuthenticated;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
};

#endif
