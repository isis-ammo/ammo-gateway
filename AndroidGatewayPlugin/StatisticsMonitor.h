#ifndef STATISTICS_MONITOR_H
#define STATISTICS_MONITOR_H

#include <string>

class AndroidEventHandler;

class StatisticsMonitor {
public:
  StatisticsMonitor();
  
  deviceConnected(AndroidEventHandler *deviceHandler);
  deviceDisconnected(AndroidEventHandler *deviceHandler);
  
  sendPeriodicUpdate();
  
  deviceListQuery();
  deviceInfoQuery(string deviceId);
  
private:
  std::multimap<std::string, AndroidEventHandler *> activeHandlers;
};

#endif //STATISTICS_MONITOR_H
