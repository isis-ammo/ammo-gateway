#ifndef NETWORK_EVENT_HANDLER_H
#define NETWORK_EVENT_HANDLER_H

#include "NetworkEnumerations.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkServiceHandler;
      
      template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkEventHandler {
      public:
        NetworkEventHandler();
        void sendMessage(ProtobufMessageWrapper *msg, char priority = 0);
        
        virtual void onConnect(std::string &peerAddress) = 0;
        virtual void onDisconnect() = 0;
        virtual int onMessageAvailable(ProtobufMessageWrapper *msg) = 0; //this method is responsible for deleting msg
        virtual int onError(const char errorCode) = 0;
        
        void close();
        void setServiceHandler(void *handler);
      private:
        NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *serviceHandler;
      };
    }
  }
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::NetworkEventHandler() : serviceHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::sendMessage(ProtobufMessageWrapper *msg, char priority) {
  serviceHandler->sendMessage(msg, priority);
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::close() {
  serviceHandler->close();
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::setServiceHandler(void *handler) {
  serviceHandler = static_cast<NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *>(handler);
}

#endif
