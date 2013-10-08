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
