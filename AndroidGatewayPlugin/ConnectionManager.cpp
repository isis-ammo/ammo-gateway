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