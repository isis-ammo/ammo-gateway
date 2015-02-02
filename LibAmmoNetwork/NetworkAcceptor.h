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

#ifndef NETWORK_ACCEPTOR_H
#define NETWORK_ACCEPTOR_H

#include <ace/Acceptor.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Acceptor.h>
#include <ace/SOCK_Stream.h>

#include "NetworkEnumerations.h"
#include "NetworkServiceHandler.h"

namespace ammo {
  namespace gateway {
    namespace internal {
      template <class ProtobufMessageWrapper, class EventHandler, SynchronizationMethod SyncMethod, unsigned int MagicNumber>
      class NetworkAcceptor {
      public:
        NetworkAcceptor(std::string serverAddress, int serverPort);
        virtual ~NetworkAcceptor();
      private:
        ACE_Acceptor<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Acceptor> *acceptor;
      };
    }
  }
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkAcceptor<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::NetworkAcceptor(std::string serverAddress, int serverPort) : acceptor(NULL) {
  ACE_INET_Addr address(serverPort, serverAddress.c_str());
  
  acceptor = new ACE_Acceptor<ammo::gateway::internal::NetworkServiceHandler<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>, ACE_SOCK_Acceptor>(address);
}

template <class ProtobufMessageWrapper, class EventHandler, ammo::gateway::internal::SynchronizationMethod SyncMethod, unsigned int MagicNumber>
ammo::gateway::internal::NetworkAcceptor<ProtobufMessageWrapper, EventHandler, SyncMethod, MagicNumber>::~NetworkAcceptor() {
  if(acceptor) {
    delete acceptor;
  }
}

#endif
