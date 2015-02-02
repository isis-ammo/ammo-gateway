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
