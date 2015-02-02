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

#ifndef GATEWAY_CONNECTION_MANAGER_H
#define GATEWAY_CONNECTION_MANAGER_H

#include <ace/Task.h>

#include "NetworkConnector.h"
#include "CrossGatewayEventHandler.h"
#include "NetworkEnumerations.h"
#include "protocol/GatewayPrivateMessages.pb.h"

class CrossGatewayEventHandler;

class CrossGatewayConnectionManager : public ACE_Task <ACE_MT_SYNCH> {
public:
  CrossGatewayConnectionManager();
  
  virtual int svc();  
  virtual int handle_input(ACE_HANDLE fd);
  
  void cancel();
  bool isCancelled();
  
private:        
  ammo::gateway::internal::NetworkConnector<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d> *connector;
  CrossGatewayEventHandler *handler;
  
  ACE_Thread_Mutex cancelMutex;
  bool cancelled;
};

#endif //GATEWAY_CONNECTION_MANAGER_H
