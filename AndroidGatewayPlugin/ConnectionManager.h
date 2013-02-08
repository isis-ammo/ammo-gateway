#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <set>
#include <ace/Event_Handler.h>

class AndroidEventHandler;

class ConnectionManager : public ACE_Event_Handler {
public:
  static const int TIMEOUT_TIME_SECONDS = 60;
  static ConnectionManager *getInstance();

  ConnectionManager();

  void registerConnection(AndroidEventHandler *handler);
  void unregisterConnection(AndroidEventHandler *handler);
  
  void checkTimeouts();
  
  //Event handler methods (called by the reactor when our timer expires)
  int handle_timeout(const ACE_Time_Value &currentTime, const void *act = 0);

private:
  static ConnectionManager *sharedInstance;
  
  typedef std::set<AndroidEventHandler *> EventHandlerSet;
  EventHandlerSet eventHandlers;
};

#endif //CONNECTION_MANAGER_H
