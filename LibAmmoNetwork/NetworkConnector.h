#ifndef NETWORK_CONNECTOR_H
#define NETWORK_CONNECTOR_H

#include <ace/Connector.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/SOCK_Stream.h>

#include "NetworkEnumerations.h"
#include "NetworkServiceHandler.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      
      template <class ProtobufMessageWrapper, class EventHandler, SynchronizationMethod SyncMethod, int MagicNumber>
      class NetworkConnector {
      public:
        NetworkConnector();
        virtual ~NetworkConnector();
        
        int connect(std::string address, int port, EventHandler *&handler);
      private:
        ACE_Connector<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Connector> *connector;
        NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber> *serviceHandler;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkConnector() : serviceHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkConnector() {
  
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
int ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::connect(std::string address, int port, EventHandler *&handler) {
  ACE_INET_Addr serverAddress(address, port);
  connector = new ACE_Connector<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Connector>();
  int status = connector->connect(serviceHandler, serverAddress);
  return status;
}

#endif
