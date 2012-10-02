#ifndef GPS_THREAD_H
#define GPS_THREAD_H

#include "ace/Task.h"
#include <ace/Date_Time.h>

#include "ace/DEV_Connector.h"
#include "ace/TTY_IO.h"
#include "ace/OS_NS_unistd.h"

#include <stdint.h>

#define DELTA_HISTORY_SAMPLES 20

class GpsThread : public ACE_Task<ACE_MT_SYNCH> {
public:
  GpsThread();
  virtual ~GpsThread();
  
  virtual int svc();
  
  void stop();
  
  int64_t getTimeDelta();
  bool getPosition(double &lat, double &lon);

private:
  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();
  
  int64_t deltaHistory[DELTA_HISTORY_SAMPLES];
  int deltaHistoryCount;
  
  int64_t timeDelta;
  ACE_Thread_Mutex timeDeltaMutex;
  void setTimeDelta(int64_t delta);
  
  double latitude;
  double longitude;
  bool fixAcquired;
  void setPosition(bool fix, double lat, double lon);

  ACE_TTY_IO serialDev;
  ACE_DEV_Connector serialConnector;
  
  bool initSerial();
  
  bool processMessage(ACE_Time_Value msgTime, std::string msg);
  
  char read_a_char();
  int write_a_char(unsigned char toWrite);
};

#endif
