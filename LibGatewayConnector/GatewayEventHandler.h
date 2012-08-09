#ifndef GATEWAY_EVENT_HANDLER_H
#define GATEWAY_EVENT_HANDLER_H

#include "NetworkEventHandler.h"
#include "protocol/GatewayPrivateMessages.pb.h"

namespace ammo {
  namespace gateway {
    class GatewayConnector;
    namespace internal {
      class GatewayEventHandler : public ammo::gateway::internal::NetworkEventHandler<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::SYNC_MULTITHREADED, 0xdeadbeef> {
      public:
        GatewayEventHandler();
        virtual ~GatewayEventHandler();

        virtual void onConnect(std::string &peerAddress);
        virtual void onDisconnect();
        virtual int onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg);
        virtual int onError(const char errorCode);
        
        void setParentConnector(ammo::gateway::GatewayConnector *parent);
      private:
        ammo::gateway::GatewayConnector *parent;
      };
    }
  }
}

#endif
