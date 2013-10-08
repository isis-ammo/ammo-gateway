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

#include "SatcomConfigurationManager.h"
#include "json/value.h"
#include "log.h"
#include <sstream>

static const char* CONFIG_FILE = "SatcomPluginConfig.json";

SatcomConfigurationManager &SatcomConfigurationManager::getInstance()
{
  static SatcomConfigurationManager sharedInstance;
  
  return sharedInstance;
}

SatcomConfigurationManager::SatcomConfigurationManager() : ConfigurationManager(CONFIG_FILE)
{
  init();
  populate();
}

void SatcomConfigurationManager::init()
{
#ifdef WIN32
  listenPort = "COM1";
#else
  listenPort = "/dev/ttyUSB0";
#endif

  baudRate = 2400;
  tokenTimeout = 5000;
  dataTimeout = 5000;
  initialState = "receiving";
  pliRelayRangeScale = 4;
  pliRelayTimeScale = 4;
}

void SatcomConfigurationManager::decode(const Json::Value& root)
{
  CM_DecodeString ( root, "listenPort", listenPort );
  CM_DecodeInt    ( root, "baudRate", baudRate);
  CM_DecodeInt    ( root, "tokenTimeout", tokenTimeout);
  CM_DecodeInt    ( root, "dataTimeout", dataTimeout);
  CM_DecodeString ( root, "initialState", initialState);
  CM_DecodeInt    ( root, "pliRelayRangeScale", pliRelayRangeScale);
  CM_DecodeInt    ( root, "pliRelayTimeScale", pliRelayTimeScale);

  LOG_INFO("Serial Plugin Configuration: ");
  LOG_INFO("  Listen Port: " << listenPort);
  LOG_INFO("  Baud Rate: " << baudRate);
  LOG_INFO("  Token Timeout: " << tokenTimeout);
  LOG_INFO("  Data Timeout: " << dataTimeout);
  LOG_INFO("  Initial State: " << initialState);
  LOG_INFO("  PLI Relay Range Scale: " << pliRelayRangeScale);
  LOG_INFO("  PLI Relay Time Scale: " << pliRelayTimeScale);
}
