
#include <string>
#include "GatewaySync.pb.h"

#ifndef GATEWAY_SYNC_SERIALIZATION_H
#define GATEWAY_SYNC_SERIALIZATION_H

namespace ammo
{
  namespace gateway
  {
    namespace serialization
    {
      class RequestChecksumsMessage;
      class SendChecksumsMessage;
      class RequestObjectsMessage;
      class SendObjectsMessage;
    }
  }
}

using namespace ammo::gateway::serialization;

class requestChecksumsMessageData
{
public:
  long tv_sec_;

  std::string encodeJson (void);
  bool decodeJson (const std::string &data);
  
  std::string encodeProtobuf (void);
  bool decodeProtobuf (const std::string &msg);
};

class sendChecksumsMessageData
{
public:
  sendChecksumsMessageData (void);
  sendChecksumsMessageData (const char *keyname);

  std::vector<std::string> checksums_;
  
  std::string encodeJson (void);
  bool decodeJson (const std::string &data);
  
  std::string encodeProtobuf (void);
  bool decodeProtobuf (const std::string &msg);

protected:
  std::string jsonKey_;
};

class requestObjectsMessageData : public sendChecksumsMessageData
{
public:
  requestObjectsMessageData (void);
};

class sendObjectsMessageData
{
public:
  struct dbRow
  {
    std::string uri_;
    std::string mime_type_;
    std::string origin_user_;
    long tv_sec_;
    long tv_usec_;
    std::string data_;
    std::string checksum_;
  };
  
  std::vector<dbRow> objects_;
  
  std::string encodeJson (void);
  bool decodeJson (const std::string &data);
  
  std::string encodeProtobuf (void);
  bool decodeProtobuf (const std::string &msg);
};

#endif // GATEWAY_SYNC_SERIALIZATION_H
