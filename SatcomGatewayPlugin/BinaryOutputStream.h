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

#ifndef BINARY_OUTPUT_STREAM_H
#define BINARY_OUTPUT_STREAM_H

#include <string>
#include <iostream>
#include <sstream>

#include "Typedefs.h"

class BinaryOutputStream {
public:
  enum StreamByteOrder {
    ORDER_LITTLE_ENDIAN,
    ORDER_BIG_ENDIAN
  };

  BinaryOutputStream(StreamByteOrder order = ORDER_LITTLE_ENDIAN);

  std::string getString();

  //Changes endianness mid-stream.  Will not affect data that has already been written.
  void changeByteOrder(StreamByteOrder order);

  void appendInt8(const int8_t value);
  void appendUInt8(const uint8_t value);

  void appendInt16(const int16_t value);
  void appendUInt16(const uint16_t value);

  void appendInt32(const int32_t value);
  void appendUInt32(const uint32_t value);

  void appendBytes(const std::string &bytes);
  void appendBytes(const char *data, size_t length);

  //Helpers for terse-specific compound types.  Stream
  //should be in big-endian mode for these to work correctly
  //(terse-encoded data is always big-endian)
  void appendTerseString(const std::string &str);
  void appendTerseBlob(const std::string &data);
  void appendTerseFile(const std::string &data);

private:
  std::ostringstream stream;
  StreamByteOrder order;
};

#endif //BINARY_OUTPUT_STREAM_H
