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
