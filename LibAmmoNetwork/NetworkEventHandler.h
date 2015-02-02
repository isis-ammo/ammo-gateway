/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
        virtual ~NetworkEventHandler();
        
        void sendMessage(ProtobufMessageWrapper *msg);
        
        virtual void onConnect(std::string &peerAddress) = 0;
        virtual void onDisconnect() = 0;
        virtual int onMessageAvailable(ProtobufMessageWrapper *msg) = 0; //this method is responsible for deleting msg
        virtual int onError(const char errorCode) = 0;
        
        void close();
        void scheduleDeferredClose();
        
        void setServiceHandler(void *handler);
      protected: //FIXME: protected to allow reactor lock hack in AndroidEventHandler (should be private)
        NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *serviceHandler;
      };
    }
  }
}


template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::NetworkEventHandler() : serviceHandler(NULL) {
  
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::~NetworkEventHandler() {
  
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::sendMessage(ProtobufMessageWrapper *msg) {
  serviceHandler->sendMessage(msg);
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::close() {
  serviceHandler->close();
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::scheduleDeferredClose() {
  serviceHandler->scheduleDeferredClose();
}

template <class ProtobufMessageWrapper, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
void ammo::gateway::internal::NetworkEventHandler<ProtobufMessageWrapper, SyncMethod, MagicNumber>::setServiceHandler(void *handler) {
  serviceHandler = static_cast<NetworkServiceHandler<ProtobufMessageWrapper, NetworkEventHandler, SyncMethod, MagicNumber> *>(handler);
}

#endif
