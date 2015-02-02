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

#ifndef SPOT_PUSH_RECEIVER_H
#define SPOT_PUSH_RECEIVER_H

#include "GatewayConnector.h"



class SpotPushReceiver : public ammo::gateway::DataPushReceiverListener, public ammo::gateway::GatewayConnectorDelegate {
public:
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  
  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);


  //PullRequestReceiverListener methods
  /*virtual void onDataReceived(ammo::gateway::GatewayConnector *sender, 
			      std::string requestUid, std::string pluginId,
			      std::string mimeType, std::string query,
			      std::string projection, unsigned int maxResults,
			      unsigned int startFromCount, bool liveQuery);*/

};

class SpotReport {
public:
  std::string content_guid;
  long report_time;
  std::string reporting_unit;
  int size;
  std::string activity;
  std::string location_utm;
  std::string enemy_unit;
  long observation_time;
  std::string unit;
  std::string equipment;
  std::string assessment;
  std::string narrative;
  std::string authentication;

};


#endif        //  #ifndef SPOT_PUSH_RECEIVER_H

