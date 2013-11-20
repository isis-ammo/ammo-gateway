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

#ifndef SERIAL_CONFIGURATION_MANAGER_H
#define SERIAL_CONFIGURATION_MANAGER_H

#include <vector>
#include "ConfigurationManager.h"

class SerialConfigurationManager : public ConfigurationManager {
public:
  static SerialConfigurationManager* getInstance();
  
  std::vector<std::string> getListenPorts() { return listenPorts; }
  std::string getGpsPort() { return gpsPort; }

  bool getSendEnabled() { return sendEnabled; }

  int getBaudRate() { return baudRate; }
  int getSlotDuration() { return slotDuration; }
  int getSlotNumber() { return slotNumber; }
  int getNumberOfSlots() { return numberOfSlots; }
  int getTransmitDuration() { return transmitDuration; }
  int getGpsTimeOffset() { return gpsTimeOffset; }

  int getPliRelayPerCycle() { return pliRelayPerCycle; }

  bool getPliRelayEnabled() { return pliRelayEnabled; }
  int getPliSendFrequency() { return pliSendFrequency; }
  std::string getPliRelayNodeName() { return pliRelayNodeName; }
  
  int getRangeScale() { return rangeScale; }
  int getTimeScale() { return timeScale; }

protected:
  void init();
  void decode(const Json::Value& root);
  
private:
  SerialConfigurationManager();
  
  static SerialConfigurationManager *sharedInstance;
  std::vector<std::string> listenPorts;
  std::string gpsPort;

  bool sendEnabled;

  int baudRate;
  int slotDuration;
  int slotNumber;
  int numberOfSlots;
  int transmitDuration;

  int gpsTimeOffset;

  int pliRelayPerCycle;

  bool pliRelayEnabled;
  int pliSendFrequency;
  std::string pliRelayNodeName;
  
  int rangeScale;
  int timeScale;
};

#endif //SERIAL_CONFIGURATION_MANAGER_H
