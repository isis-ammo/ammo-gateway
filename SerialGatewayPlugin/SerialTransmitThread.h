/*
 * SerialTransmitThread.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef SERIALTRANSMITTHREAD_H_
#define SERIALTRANSMITTHREAD_H_

#include <ace/Task.h>

class SerialServiceHandler;
class GatewayReceiver;
class GpsThread;

class SerialTransmitThread: public ACE_Task<ACE_MT_SYNCH> {
public:
  SerialTransmitThread(SerialServiceHandler *parent, GatewayReceiver *receiver, GpsThread *gpsThread);
  virtual ~SerialTransmitThread();

  virtual int svc();

  void stop();

private:
  SerialServiceHandler *parent;
  GatewayReceiver *receiver;
  GpsThread *gpsThread;

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();

  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;
  int gpsTimeOffset;


  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
};

#endif /* SERIALTRANSMITTHREAD_H_ */
