/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA.
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
