#ifndef PASS_AMMO_PUBLISHER_H
#define PASS_AMMO_PUBLISHER_H

namespace ammo
{
  namespace gateway
  {
    class GatewayConnector;
  }
}

class PassAmmmoPublisher
{
public:
  static ammo::gateway::GatewayConnector *connector;
  
  static void pushPli (const std::string &lid,
                       const std::string &userid,
                       const std::string &unitid,
                       const std::string &name,
                       const std::string &lat,
                       const std::string &lon,
                       const std::string &created,
                       const std::string &modified);
};

#endif // PASS_AMMO_PUBLISHER_H
