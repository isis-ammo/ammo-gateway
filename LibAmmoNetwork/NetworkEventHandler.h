#ifndef NETWORK_EVENT_HANDLER_H
#define NETWORK_EVENT_HANDLER_H

namespace ammo {
  namespace gateway {
    namespace internal {
      template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
      class NetworkServiceHandler;
      
      template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
      class NetworkEventHandler {
      public:
        NetworkEventHandler();
        void sendMessage(ProtobufMessageWrapper *msg, char priority);
        
        virtual void onConnect(std::string &peerAddress) = 0;
        virtual void onDisconnect() = 0;
        virtual int onMessageAvailable(ProtobufMessageWrapper *msg) = 0; //this method is responsible for deleting msg
        virtual int onError(const char errorCode) = 0;
        
      private:
        void setServiceHandler(NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *handler);
        
        NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *serviceHandler;
      };
    }
  }
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::NetworkEventHandler() : serviceHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::sendMessage(ProtobufMessageWrapper *msg, char priority) {
  serviceHandler->sendMessage(msg, priority);
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::setServiceHandler(ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *handler) {
  serviceHandler = handler;
}

#endif
