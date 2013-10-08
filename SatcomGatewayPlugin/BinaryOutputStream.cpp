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

#define NOMINMAX //When building under visual studio, a max macro defined in winnt.h 
                 //interferes with the members of numeric_limits

#include "BinaryOutputStream.h"

#include <limits>

#include <log.h>

/*
 * TODO: This stream makes the assumption that we're on a little-endian
 * machine.  Eventually, we should fix that.
 */

#define htonll(x) (((int64_t)(ntohl((int32_t)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((int)(x >> 32))))

BinaryOutputStream::BinaryOutputStream(BinaryOutputStream::StreamByteOrder order) : 
stream(),
order(order) {
  //do nothing
}

std::string BinaryOutputStream::getString() {
  return stream.str();
}

//Changes endianness mid-stream.  Will not affect data that has already been written.
void BinaryOutputStream::changeByteOrder(BinaryOutputStream::StreamByteOrder newOrder) {
  order = newOrder;
}

void BinaryOutputStream::appendInt8(const int8_t value) {
  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendUInt8(const uint8_t value) {
  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendInt16(const int16_t value) {
  int16_t streamVal = value;
  if(order == ORDER_BIG_ENDIAN) {
    streamVal = htons(value);
  }

  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendUInt16(const uint16_t value) {
  uint16_t streamVal = value;
  if(order == ORDER_BIG_ENDIAN) {
    streamVal = htons(value);
  }

  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendInt32(const int32_t value) {
  uint32_t streamVal = value;
  if(order == ORDER_BIG_ENDIAN) {
    streamVal = htonl(value);
  }

  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendUInt32(const uint32_t value) {
  uint32_t streamVal = value;
  if(order == ORDER_BIG_ENDIAN) {
    streamVal = htonl(value);
  }

  stream.write(reinterpret_cast<const char *>(&value), sizeof(value));
}

void BinaryOutputStream::appendBytes(const std::string &bytes) {
  stream << bytes;
}

void BinaryOutputStream::appendBytes(const char *data, size_t length) {
  stream.write(data, length);
}

void BinaryOutputStream::appendTerseString(const std::string &str) {
  if(str.length() > std::numeric_limits<uint16_t>::max()) {
    LOG_WARN("String too long; putting zero-length string instead");
    appendUInt16(0);
  } else {
    appendUInt16(str.length());
    appendBytes(str);
  }
}

void BinaryOutputStream::appendTerseBlob(const std::string &data) {
  //blob format is currently identical to terse string format (it might
  //change later, which is why this method exists)
  appendTerseString(data);
}

void BinaryOutputStream::appendTerseFile(const std::string &data) {
  if(data.length() > std::numeric_limits<uint32_t>::max()) {
    LOG_WARN("File data too long; putting zero-length file instead");
    appendUInt32(0);
  } else {
    appendUInt32(data.length());
    appendBytes(data);
  }
}
