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
      
      template <class ProtobufMessageWrapper, class EventHandler, SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkConnector {
      public:
        NetworkConnector();
        virtual ~NetworkConnector();
        
        int connect(std::string address, int port, EventHandler *&handler);
        void close();
      private:
        ACE_Connector<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Connector> *connector;
        NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber> *serviceHandler;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkConnector() : connector(NULL), serviceHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkConnector() {
  if(connector) {
    delete connector;
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
int ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::connect(std::string address, int port, EventHandler *&handler) {
  ACE_INET_Addr serverAddress(port, address.c_str());
  connector = new ACE_Connector<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Connector>();
  int status = connector->connect(serviceHandler, serverAddress);
  if(status == 0 && serviceHandler) {
    LOG_TRACE("NetworkConnector assigning EventHandler to handler");
    handler = dynamic_cast<EventHandler *>(serviceHandler->eventHandler);
  }
  return status;
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkConnector<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::close() {
  if(connector) {
    connector->close();
  }
}

#endif
