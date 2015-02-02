/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
