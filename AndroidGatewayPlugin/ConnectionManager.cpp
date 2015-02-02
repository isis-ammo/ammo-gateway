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

#include <ace/Reactor.h>
#include <ace/Select_Reactor.h>

#include "ConnectionManager.h"
#include "AndroidEventHandler.h"
#include "log.h"

ConnectionManager* ConnectionManager::sharedInstance = NULL;

ConnectionManager* ConnectionManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new ConnectionManager();
  }
  
  return sharedInstance;
}

ConnectionManager::ConnectionManager() : eventHandlers(){

}

void ConnectionManager::registerConnection(AndroidEventHandler *handler) {
  eventHandlers.insert(handler);
}

void ConnectionManager::unregisterConnection(AndroidEventHandler *handler) {
  EventHandlerSet::size_type numberRemoved = eventHandlers.erase(handler);
  if(numberRemoved == 0) {
    LOG_WARN("Event handler " << handler << " not found in connection manager map");
  }
}

void ConnectionManager::checkTimeouts() {
  LOG_DEBUG("Checking for timed-out connections...");
  for(EventHandlerSet::iterator it = eventHandlers.begin(); it != eventHandlers.end(); it++) {
    (*it)->checkTimeout();
  }
}

int ConnectionManager::handle_timeout(const ACE_Time_Value &currentTime, const void *act) {
  LOG_DEBUG("ConnectionManager woke up @ " << currentTime);
  
  checkTimeouts();
  
  //reset our timer so we run again in a little while
  ACE_Reactor::instance()->schedule_timer(this, NULL, ACE_Time_Value(ConnectionManager::TIMEOUT_TIME_SECONDS));
  return 0;
}