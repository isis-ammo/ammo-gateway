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
