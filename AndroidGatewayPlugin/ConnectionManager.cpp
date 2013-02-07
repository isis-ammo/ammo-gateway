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