#include "ConnectionManager.h"
#include "AndroidEventHandler.h"

ConnectionManager* ConnectionManager::sharedInstance = NULL;

ConnectionManager* ConnectionManager::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new ConnectionManager();
  }
  
  return sharedInstance;
}

ConnectionManager::ConnectionManager() {

}

void registerConnection(AndroidEventHandler *handler) {

}

void unregisterConnection(AndroidEventHandler *handler) {

}