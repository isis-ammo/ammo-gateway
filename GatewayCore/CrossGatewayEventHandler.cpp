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

#include <string>

#include "CrossGatewayEventHandler.h"
#include "NetworkServiceHandler.h"
#include "GatewayCore.h"
#include "GatewayConfigurationManager.h"
#include "log.h"



using namespace std;
using namespace ammo::gateway::internal;

void CrossGatewayEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("CrossGatewayEventHandler::onConnect(" << peerAddress << ")");
  LOG_INFO("Got cross gateway connection from " << peerAddress << ")");
  
  gatewayId = "";
  gatewayIdAuthenticated = false;
  registeredWithGateway = false;
  
  //send an authentication message to the other gateway
  ammo::gateway::protocol::GatewayWrapper *newMsg = new ammo::gateway::protocol::GatewayWrapper();
  newMsg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_CROSS_GATEWAY);
  newMsg->set_message_priority(PRIORITY_AUTH);
  newMsg->mutable_associate_cross_gateway()->set_gateway_id(GatewayConfigurationManager::getInstance()->getCrossGatewayId());
  newMsg->mutable_associate_cross_gateway()->set_key(GatewayConfigurationManager::getInstance()->getCrossGatewayId());
  LOG_DEBUG("Sending associate message to connected gateway...");
  this->sendMessage(newMsg);
}

void CrossGatewayEventHandler::onDisconnect() {
  LOG_TRACE("CrossGatewayEventHandler::onDisconnect()");
}

int CrossGatewayEventHandler::onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg) {
  LOG_TRACE("CrossGatewayEventHandler::onMessageAvailable()");
  
  if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_CROSS_GATEWAY) {
    LOG_DEBUG("Received Associate CrossGateway...");
    //TODO: split out into a different function and do more here
    ammo::gateway::protocol::GatewayWrapper *newMsg = new ammo::gateway::protocol::GatewayWrapper();
    newMsg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT);
    newMsg->set_message_priority(PRIORITY_AUTH);
    newMsg->mutable_associate_result()->set_result(ammo::gateway::protocol::AssociateResult_Status_SUCCESS);
    this->sendMessage(newMsg);
    gatewayId = msg->associate_cross_gateway().gateway_id();
    gatewayIdAuthenticated = true;
    GatewayCore::getInstance()->registerCrossGatewayConnection(gatewayId, this);
    registeredWithGateway = true;
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    //TODO: Handle remote auth success/failure
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST) {
    LOG_DEBUG("Received Register Data Interest...");
    std::string mime_type = msg->register_data_interest().mime_type();
    bool result = GatewayCore::getInstance()->subscribeCrossGateway(mime_type, gatewayId);
    if(result == true) {
      registeredHandlers.push_back(mime_type);
    }
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST) {
    LOG_DEBUG("Received Unregister Data Interest...");
    std::string mime_type = msg->unregister_data_interest().mime_type();
    bool result = GatewayCore::getInstance()->unsubscribeCrossGateway(mime_type, gatewayId);
    if(result == true) {
      for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
        if((*it) == mime_type) {
          registeredHandlers.erase(it);
          break;
        }
      }
    }
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    uint64_t receivedTime = msg->push_data().received_time();
    if(receivedTime == 0) {
      //Reject messages with unset received timestamp, because a missing value will break datastore sync
      LOG_ERROR("PushData: received_time missing...  rejecting message!");
      LOG_ERROR("          To resolve this error, update the connected gateway.");
    } else {
      GatewayCore::getInstance()->pushCrossGateway(msg->push_data().uri(), msg->push_data().mime_type(), msg->push_data().encoding(), msg->push_data().data(), msg->push_data().origin_user(), gatewayId, msg->message_priority(), msg->push_data().received_time());
    }
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request...");
    bool result = GatewayCore::getInstance()->pullRequestCrossGateway(msg->pull_request().request_uid(), msg->pull_request().plugin_id(), msg->pull_request().mime_type(), msg->pull_request().query(),
                                                        msg->pull_request().projection(), msg->pull_request().max_results(), msg->pull_request().start_from_count(),
                                                        msg->pull_request().live_query(), gatewayId, msg->message_priority());
    if(result == true) {
      registeredPullResponsePluginIds.insert(msg->pull_request().plugin_id());
    }
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE) {
    LOG_DEBUG("Received Pull Response...");
    GatewayCore::getInstance()->pullResponseCrossGateway(msg->pull_response().request_uid(), msg->pull_response().plugin_id(), msg->pull_response().mime_type(),
                                                         msg->pull_response().uri(), msg->pull_response().encoding(), msg->pull_response().data(), gatewayId, msg->message_priority());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST) {
    LOG_DEBUG("Received Register Pull Interest...");
    std::string mime_type = msg->register_pull_interest().mime_type();
    bool result = GatewayCore::getInstance()->registerPullInterestCrossGateway(mime_type, gatewayId);
    if(result == true) {
      registeredPullHandlers.push_back(mime_type);
    }
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST) {
    LOG_DEBUG("Received Unregister Pull Interest...");
    std::string mime_type = msg->unregister_pull_interest().mime_type();
    bool result = GatewayCore::getInstance()->unregisterPullInterestCrossGateway(mime_type, gatewayId);
    if(result == true) {
      for(std::vector<std::string>::iterator it = registeredPullHandlers.begin(); it != registeredPullHandlers.end(); it++) {
        if((*it) == mime_type) {
          registeredPullHandlers.erase(it);
          break;
        }
      }
    }
  }
  
  delete msg;  
  
  return 0;
}

int CrossGatewayEventHandler::onError(const char errorCode) {
  LOG_ERROR("CrossGatewayEventHandler::onError(" << errorCode << ")");
  return 0;
}

bool CrossGatewayEventHandler::sendSubscribeMessage(std::string mime_type) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RegisterDataInterest *subscribeMsg = msg->mutable_register_data_interest();
  subscribeMsg->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendUnsubscribeMessage(std::string mime_type) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterDataInterest *unsubscribeMsg = msg->mutable_unregister_data_interest();
  unsubscribeMsg->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, char priority, uint64_t receivedTime) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  pushMsg->set_encoding(encoding);
  pushMsg->set_origin_user(originUser);
  pushMsg->set_received_time(receivedTime);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  msg->set_message_priority(priority);
  
  LOG_DEBUG("Sending Data Push message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, std::string query, 
                                                 std::string projection, unsigned int maxResults, unsigned int startFromCount, bool liveQuery, char priority) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PullRequest *pullMsg = msg->mutable_pull_request();
  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_query(query);
  pullMsg->set_projection(projection);
  pullMsg->set_max_results(maxResults);
  pullMsg->set_start_from_count(startFromCount);
  pullMsg->set_live_query(liveQuery);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST);
  msg->set_message_priority(priority);
  
  LOG_DEBUG("Sending Pull Request message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType,
                                                  std::string uri, std::string encoding, const std::string& data, char priority) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PullResponse *pullRsp = msg->mutable_pull_response();
  pullRsp->set_request_uid(requestUid);
  pullRsp->set_plugin_id(pluginId);
  pullRsp->set_mime_type(mimeType);
  pullRsp->set_uri(uri);
  pullRsp->set_encoding(encoding);
  pullRsp->set_data(data);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE);
  msg->set_message_priority(priority);
  
  LOG_DEBUG("Sending Pull Response message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}


bool CrossGatewayEventHandler::sendRegisterPullInterest(std::string mimeType) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RegisterPullInterest *di = msg->mutable_register_pull_interest();
  di->set_mime_type(mimeType);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Register Pull Interest message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendUnregisterPullInterest(std::string mimeType) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterPullInterest *di = msg->mutable_unregister_pull_interest();
  di->set_mime_type(mimeType);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Unregister Pull Interest message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

CrossGatewayEventHandler::~CrossGatewayEventHandler() {
  LOG_DEBUG("CrossGatewayEventHandler being destroyed!");
  
  LOG_DEBUG("Unregistering data handlers...");
  for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
    GatewayCore::getInstance()->unsubscribeCrossGateway(*it, gatewayId);
  }
  
  LOG_DEBUG("Unregistering pull request handlers...");
  for(std::vector<std::string>::iterator it = registeredPullHandlers.begin(); it != registeredPullHandlers.end(); it++) {
    GatewayCore::getInstance()->unregisterPullInterestCrossGateway(*it, gatewayId);
  }
  
  LOG_DEBUG("Unregistering pull response plugin IDs...");
  for(std::set<std::string>::iterator it = registeredPullResponsePluginIds.begin(); it != registeredPullResponsePluginIds.end(); it++) {
    GatewayCore::getInstance()->unregisterPullResponsePluginIdCrossGateway(*it, gatewayId);
  }
  
  if(registeredWithGateway) {
    GatewayCore::getInstance()->unregisterCrossGatewayConnection(gatewayId);
  }
}
