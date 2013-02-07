#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <set>

class AndroidEventHandler;

class ConnectionManager {
public:
  static ConnectionManager *getInstance();

  ConnectionManager();

  void registerConnection(AndroidEventHandler *handler);
  void unregisterConnection(AndroidEventHandler *handler);

private:
  static ConnectionManager *sharedInstance;
  
  typedef std::set<AndroidEventHandler *> EventHandlerSet;
  EventHandlerSet eventHandlers;
};

#endif //CONNECTION_MANAGER_H
