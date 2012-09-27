/*
 * SerialTransmitThread.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef SERIALTRANSMITTHREAD_H_
#define SERIALTRANSMITTHREAD_H_

#include <ace/Task.h>
#include <stdint.h>

class SerialServiceHandler;
class GatewayReceiver;
class GpsThread;

struct TerseMessageHeader {
  uint8_t  magic[3];
  uint8_t  versionAndSlot; //first two bits set to 01 and next six bits encode phone ID
  uint16_t payloadSize;
  uint16_t payloadChecksum;
  uint32_t timestamp;
  uint16_t reserved;
  uint16_t headerChecksum;
};

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

  void sendMessage(std::string *msg);

  int baudRate;
  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;
  int gpsTimeOffset;
  int pliSendFrequency;


  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
};

#endif /* SERIALTRANSMITTHREAD_H_ */
