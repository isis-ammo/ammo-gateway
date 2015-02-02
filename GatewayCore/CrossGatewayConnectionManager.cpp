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

#include <ace/OS_NS_unistd.h>

#include "CrossGatewayConnectionManager.h"
#include "CrossGatewayEventHandler.h"
#include "GatewayCore.h"
#include "GatewayConfigurationManager.h"
#include "log.h"


using namespace ammo::gateway::internal;

const int SLEEP_TIME = 3;

CrossGatewayConnectionManager::CrossGatewayConnectionManager() : connector(NULL), handler(NULL), cancelMutex(), cancelled(false) {
  
}
  
int CrossGatewayConnectionManager::svc() {
  GatewayConfigurationManager *config = GatewayConfigurationManager::getInstance();
  
  bool connected = false;
  int connectionAttempt = 1;
  handler = NULL;
  
  while(!connected && !isCancelled()) {
    LOG_INFO("Attempting connection...  attempt " << connectionAttempt);
    connector = new ammo::gateway::internal::NetworkConnector<ammo::gateway::protocol::GatewayWrapper, CrossGatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0x8badf00d>();
  
    int status = connector->connect(config->getCrossGatewayParentAddress(), config->getCrossGatewayParentPort(), handler);
    if(status == -1) {
      LOG_ERROR("Connection to gateway failed (" << errno << ": " << strerror(errno) << ")" );
      connectionAttempt++;
      connected = false;
      delete connector;
      connector = NULL;
      handler = NULL;
      ACE_OS::sleep(SLEEP_TIME);
    } else {
      LOG_INFO("Connection to gateway established.");
      connected = true;
      ACE_Reactor *reactor = ACE_Reactor::instance();
      reactor->notify(this, ACE_Event_Handler::READ_MASK); //runs handle_input in the reactor's thread (so we don't have
                                                           //to worry about a bunch of synchronization junk).
    }
  }
  return 0;
}

int CrossGatewayConnectionManager::handle_input(ACE_HANDLE fd) {
  //Tell the GatewayCore which handler we created, so it can restart the
  //connection loop when it disconnects (once connections are established,
  //there's no other way to tell which handler is our parent and which are
  //children)
  GatewayCore::getInstance()->setParentHandler(handler);
  return 0;
}

void CrossGatewayConnectionManager::cancel() {
  cancelMutex.acquire();
  cancelled = true;
  cancelMutex.release();
}

bool CrossGatewayConnectionManager::isCancelled() {
  cancelMutex.acquire();
  volatile bool ret = cancelled;
  cancelMutex.release();
  return ret;
}
