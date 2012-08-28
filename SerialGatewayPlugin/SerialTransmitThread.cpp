/*
 * SerialTransmitThread.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#include "SerialTransmitThread.h"
#include "SerialConfigurationManager.h"
#include "GpsThread.h"

#include "ace/Time_Value.h"
#include <cmath>

const int BAUD_RATE = 9600;

SerialTransmitThread::SerialTransmitThread(SerialServiceHandler *parent, GatewayReceiver *receiver, GpsThread *gpsThread) : parent(parent), receiver(receiver), gpsThread(gpsThread), closed(false), newMessageAvailable(newMessageMutex) {
  // TODO Auto-generated constructor stub
  SerialConfigurationManager *config = SerialConfigurationManager::getInstance();
  slotDuration = config->getSlotDuration();
  slotNumber = config->getSlotNumber();
  numberOfSlots = config->getNumberOfSlots();
  transmitDuration = config->getTransmitDuration();
  gpsTimeOffset = config->getGpsTimeOffset();
}

SerialTransmitThread::~SerialTransmitThread() {
  // TODO Auto-generated destructor stub
}

void SerialTransmitThread::stop() {
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
}

int SerialTransmitThread::svc() {
  int offset = ((slotNumber - 1) % numberOfSlots) * slotDuration;
  int cycleDuration = slotDuration * numberOfSlots;
  double bytesPerMs = BAUD_RATE / (10*1000.0);

  long tweakedTransmitDuration = transmitDuration + (long) std::min(50, transmitDuration / 10);
  long maxPayloadSize = (long) (transmitDuration * bytesPerMs);

  while(!isClosed()) {
    long systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
    long gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;

    long cycleStartTime = (long) (gpsTime / cycleDuration) * cycleDuration;
    long thisSlotBegin = cycleStartTime + offset;
    long thisSlotEnd = thisSlotBegin + tweakedTransmitDuration;

    long thisSlotConsumed = 0;

    if(gpsTime < thisSlotBegin) {
      //sleep until slot begins
      usleep((thisSlotBegin - gpsTime) * 1000);
    } else if(gpsTime > thisSlotEnd) {
      //missed our slot...  sleep until next slot begins
      usleep((thisSlotBegin + cycleDuration - gpsTime) * 1000);
    } else {
      //we're in our slot...  send as much data as we can
    }
  }
  return 0;
}

bool SerialTransmitThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}
