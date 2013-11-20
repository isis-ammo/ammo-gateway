/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
 */

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

  void sendMessage(std::string *msg, int cycleDuration, uint8_t slotNumber, uint8_t slotIndex);

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
