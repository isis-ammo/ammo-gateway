#ifndef TERSE_DECODER_H
#define TERSE_DECODER_H

#include <cstdint>
#include <string>

#include <GatewayConnector.h>


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

  std::string parseTerseData(int mt, const char *data, size_t terseLength, std::string &originUser );
  void parseGroupPliBlob(std::string groupPliBlob, int32_t baseLat, int32_t baseLon, uint32_t baseTime);
  std::string extractString(const char *terse, size_t& cursor, size_t length);
  std::string extractOldStyleString(const char *terse, size_t& cursor, size_t length);
  std::string extractBlob(const char *terse, size_t& cursor, size_t length);
  int8_t extractInt8(const char *terse, size_t& cursor, size_t length);
  int16_t extractInt16(const char *terse, size_t& cursor, size_t length);
  int32_t extractInt32(const char *terse, size_t& cursor, size_t length);
  int64_t extractInt64(const char *terse, size_t& cursor, size_t length);
  
  std::string generateTransappsPli(std::string originUser, int32_t lat, int32_t lon, uint32_t created, int8_t hopCount);
};

#endif