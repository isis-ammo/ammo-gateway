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