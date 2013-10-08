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

#ifndef TERSE_DECODER_H
#define TERSE_DECODER_H

#include <string>

#include <GatewayConnector.h>

#include "Typedefs.h"


class TerseDecoder
{
public:
  TerseDecoder(ammo::gateway::GatewayConnector *connector);
  ~TerseDecoder();

  bool processMessage(uint8_t dataType, const std::string &terseData);

private:
  ammo::gateway::GatewayConnector *connector;

  typedef std::map<std::string, int64_t> TimestampMap;
  TimestampMap latestPliTimestamps;

  int rangeScale;
  int timeScale;

  std::string processDashMedia(const std::string &terseData);

  std::string parseTerseData(int mt, const char *data, size_t terseLength, std::string &originUser );
  void parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime);
  std::string extractString(const char *terse, size_t& cursor, size_t length);
  std::string extractOldStyleString(const char *terse, size_t& cursor, size_t length);
  std::tr1::shared_ptr<const std::string> extractFile(const char *terse, size_t &cursor, size_t length);
  std::string extractBlob(const char *terse, size_t& cursor, size_t length);
  int8_t extractInt8(const char *terse, size_t& cursor, size_t length);
  int16_t extractInt16(const char *terse, size_t& cursor, size_t length);
  int32_t extractInt32(const char *terse, size_t& cursor, size_t length);
  int64_t extractInt64(const char *terse, size_t& cursor, size_t length);
  
  std::string generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created, int8_t hopCount);
};

#endif
