#include "BinaryOutputStream.h"

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
