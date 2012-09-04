#ifndef GPS_THREAD_H
#define GPS_THREAD_H

#include "ace/Task.h"
#include <ace/Date_Time.h>
#include <stdint.h>

#define DELTA_HISTORY_SAMPLES 20

class GpsThread : public ACE_Task<ACE_MT_SYNCH> {
public:
  GpsThread();
  virtual ~GpsThread();
  
  virtual int svc();
  
  void stop();
  
  int64_t getTimeDelta();

private:
  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();
  
  int64_t deltaHistory[DELTA_HISTORY_SAMPLES];
  int deltaHistoryCount;
  
  int64_t timeDelta;
  ACE_Thread_Mutex timeDeltaMutex;
  void setTimeDelta(int64_t delta);

#ifdef WIN32
  HANDLE hComm;
#else
  int gFD;
#endif
  
  bool initSerial();
  
  bool processMessage(ACE_Time_Value msgTime, std::string msg);
  
  char read_a_char();
  int write_a_char(unsigned char toWrite);
};

#endif
