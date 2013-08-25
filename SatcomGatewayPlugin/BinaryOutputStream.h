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

private:
  std::ostringstream stream;
  StreamByteOrder order;
};

#endif //BINARY_OUTPUT_STREAM_H
