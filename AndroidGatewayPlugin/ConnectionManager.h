#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

class AndroidEventHandler;

class ConnectionManager {
public:
  static ConnectionManager *getInstance();

  ConnectionManager();

  void registerConnection(AndroidEventHandler *handler);
  void unregisterConnection(AndroidEventHandler *handler);

private:
  static ConnectionManager *sharedInstance;
};

#endif //CONNECTION_MANAGER_H
