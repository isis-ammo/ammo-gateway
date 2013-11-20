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
