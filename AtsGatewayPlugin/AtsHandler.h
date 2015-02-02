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

#ifndef ATS_HANDLER_H
#define ATS_HANDLER_H

#include <curl/curl.h>

#include "GatewayConnector.h"
#include "AtsMessageTypes.h"
#include "AtsConfigMgr.h"

class AtsHandler : 
  public ammo::gateway::DataPushReceiverListener, 
  public ammo::gateway::PullRequestReceiverListener,
  public ammo::gateway::PullResponseReceiverListener,
  public ammo::gateway::GatewayConnectorDelegate 
{
public:
  AtsHandler(); 
  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);
  
  // DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender,
                              ammo::gateway::PushData &pushData);

  // PullRequestReceiverListener methods
  virtual void onPullRequestReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PullRequest &pullReq);

  // PullResponseReceiverListener
  virtual void onPullResponseReceived (ammo::gateway::GatewayConnector *sender, ammo::gateway::PullResponse &response);
private:
  char* baseServerAddr;
  AtsConfigMgr* config;
  std::pair<std::string, std::string> credentials;

  std::string uploadMedia(CURL *curl, std::string mediaType, std::string& payload ); 
  std::string inviteChat(CURL *curl, std::string mediaType, std::string& payload ); 
  std::string listChannels(CURL *curl, std::string dataType, std::string query );
  std::string listUnits(CURL *curl, std::string dataType, std::string query );
  std::string listMembers(CURL *curl, std::string dataType, std::string query );
  std::string listLocations(CURL *curl, std::string dataType, std::string query );
  std::string listPeople(CURL *curl, std::string dataType, std::string query ); 
  std::string listPeople(CURL *curl, std::string dataType, std::vector<char>& query ); 
  std::string channelCreate(CURL *curl, std::string dataType, std::string& payload ); 

  std::string centerMap(CURL *curl, std::string dataType, std::string &query ); 

  std::string postLocation(CURL *curl, std::string mediaType, std::string& payload ); 
  std::string postLocations(CURL *curl, std::string mediaType, std::string& payload ); 
};

#endif        //  #ifndef ATS_HANDLER_H

