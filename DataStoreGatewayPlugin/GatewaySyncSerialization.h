
#include <string>
#include "GatewaySync.pb.h"

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
  
  RequestChecksumsMessage *encodeProtobuf (void);
  bool decodeProtobuf (const RequestChecksumsMessage *msg);
};

class sendChecksumsMessageData
{
public:
  sendChecksumsMessageData (void);
  sendChecksumsMessageData (const char *keyname);

  std::vector<std::string> checksums_;
  
  std::string encodeJson (void);
  bool decodeJson (const std::string &data);
  
  SendChecksumsMessage *encodeProtobuf (void);
  bool decodeProtobuf (const SendChecksumsMessage *msg);

protected:
  std::string jsonKey_;
};

class requestObjectsMessageData : public sendChecksumsMessageData
{
public:
  requestObjectsMessageData (void);
};
