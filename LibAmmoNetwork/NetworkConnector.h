#ifndef NETWORK_CONNECTOR_H
#define NETWORK_CONNECTOR_H

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
        NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber> *serviceHandler;
        EventHandler *eventHandler;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkConnector() : serviceHandler(NULL), eventHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkConnector() {
  
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
int ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::connect(std::string address, int port, EventHandler *&handler) {
  return 0;
}

#endif
