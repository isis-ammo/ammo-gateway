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
 * GatewayReceiver.h
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#ifndef GATEWAYRECEIVER_H_
#define GATEWAYRECEIVER_H_

#include "GatewayConnector.h"
#include <stdint.h>
#include <map>

class GpsThread;

struct PliInfo {
  int32_t lat;
  int32_t lon;
  int32_t createdTime;
};

class GatewayReceiver : public ammo::gateway::DataPushReceiverListener {
public:
  GatewayReceiver(GpsThread *gpsThread);
  virtual ~GatewayReceiver();

  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);

  std::string *getNextReceivedMessage();
  void addReceivedMessage(std::string *msg);

  int getNextMessageSize();
  bool isMessageAvailable();

  std::string getNextPliRelayPacket();

  void appendString(std::ostringstream &stream, const std::string &str);

  void appendInt64(std::ostringstream &stream, const int64_t val);

  void appendInt32(std::ostringstream &stream, const int32_t val);
  void appendUInt32(std::ostringstream &stream, const uint32_t val);

  void appendInt16(std::ostringstream &stream, const int16_t val);
  void appendUInt16(std::ostringstream &stream, const uint16_t val);

  void appendInt8(std::ostringstream &stream, const int8_t val);

  void appendBlob(std::ostringstream &stream, const std::string &blob);

  void addPli(std::string &username, int32_t lat, int32_t lon, int32_t createdTime);

private:
  GpsThread *gpsThread;
  
  ACE_Thread_Mutex receiveQueueMutex;

  typedef std::queue<std::string *> MessageQueue;
  MessageQueue receiveQueue;

  unsigned long long receivedMessageCount;

  typedef std::map<std::string, PliInfo> PliMap;
  PliMap pliMap;
  ACE_Thread_Mutex pliMapMutex;

  bool pliRelayEnabled;
  int pliRelayPerCycle;
  int pliIndex;
  std::string pliRelayNodeName;
  
  int rangeScale;
  int timeScale;
};

#endif /* GATEWAYRECEIVER_H_ */
