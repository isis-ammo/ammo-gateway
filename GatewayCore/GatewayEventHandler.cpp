#include <string>

#include "GatewayEventHandler.h"
#include "NetworkServiceHandler.h"
#include "GatewayCore.h"
#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

void GatewayEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("GatewayEventHandler::onConnect(" << peerAddress << ")");
  LOG_INFO("Got plugin connection from " << peerAddress << ")");
}

void GatewayEventHandler::onDisconnect() {
  LOG_TRACE("GatewayEventHandler::onDisconnect()");
}

int GatewayEventHandler::onMessageAvailable(ammo::gateway::protocol::GatewayWrapper *msg) {
  LOG_TRACE("GatewayEventHandler::onMessageAvailable()");
  
  switch(msg->type()) {
    case ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE: {
      LOG_DEBUG("Received Associate Device...");
      //TODO: split out into a different function and do more here
      ammo::gateway::protocol::GatewayWrapper *newMsg = new ammo::gateway::protocol::GatewayWrapper();
      newMsg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT);
      newMsg->set_message_priority(PRIORITY_AUTH);
      newMsg->mutable_associate_result()->set_result(ammo::gateway::protocol::AssociateResult_Status_SUCCESS);
      this->sendMessage(newMsg);
      username = msg->associate_device().user();
      usernameAuthenticated = true;
      break;
    }
    case ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST: {
      LOG_DEBUG("Received Register Data Interest...");
      std::string mime_type = msg->register_data_interest().mime_type();
      MessageScope scope;
      if(msg->register_data_interest().scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      bool result = GatewayCore::getInstance()->registerDataInterest(mime_type, scope, this);
      if(result == true) {
        registeredHandlers.push_back(mime_type);
      }
      break;
    }
    case ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST: {
      LOG_DEBUG("Received Unregister Data Interest...");
      std::string mime_type = msg->unregister_data_interest().mime_type();
      MessageScope scope;
      if(msg->unregister_data_interest().scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      bool result = GatewayCore::getInstance()->unregisterDataInterest(mime_type, scope, this);
      if(result == true) {
        for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end();) {
          if((*it) == mime_type) {
            it = registeredHandlers.erase(it); //erase returns the iterator to the next element
            break;
          } else {
            it++;
          }
        }
      }
      break;
    } 
    case ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA: {
      LOG_DEBUG("Received Push Data...");
      MessageScope scope;
      if(msg->push_data().scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      GatewayCore::getInstance()->pushData(this, msg->push_data().uri(), msg->push_data().mime_type(), msg->push_data().encoding(), msg->push_data().data(), msg->push_data().origin_user()/*this->username*/, scope, msg->message_priority());
      break;
    } 
    case ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST: {
      LOG_DEBUG("Received Pull Request...");
      LOG_TRACE("  " << msg->DebugString());
      
      ammo::gateway::protocol::PullRequest pullMsg = msg->pull_request();
      MessageScope scope;
      if(pullMsg.scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      bool result = GatewayCore::getInstance()->pullRequest(this, pullMsg.request_uid(), pullMsg.plugin_id(), pullMsg.mime_type(), pullMsg.query(),
        pullMsg.projection(), pullMsg.max_results(), pullMsg.start_from_count(), pullMsg.live_query(), scope, msg->message_priority());
      if(result == true) {
        registeredPullResponsePluginIds.insert(pullMsg.plugin_id());
      }
      break;
    } 
    case ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE: {
      LOG_DEBUG("Received Pull Response...");
      LOG_TRACE("  " << msg->DebugString());
      
      ammo::gateway::protocol::PullResponse pullRsp = msg->pull_response();
      
      GatewayCore::getInstance()->pullResponse( pullRsp.request_uid(), pullRsp.plugin_id(), pullRsp.mime_type(), pullRsp.uri(), pullRsp.encoding(), pullRsp.data(), msg->message_priority());
      break;
    }
    case ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST: {
      LOG_DEBUG("Received Register Pull Interest...");
      std::string mime_type = msg->register_pull_interest().mime_type();
      
      MessageScope scope;
      if(msg->register_pull_interest().scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      
      bool result = GatewayCore::getInstance()->registerPullInterest(mime_type, scope, this);
      if(result == true) {
        registeredPullRequestHandlers.push_back(mime_type);
      }
      break;
    } 
    case ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST: {
      LOG_DEBUG("Received Unregister Pull Interest...");
      std::string mime_type = msg->unregister_pull_interest().mime_type();
      
      MessageScope scope;
      if(msg->unregister_pull_interest().scope() == ammo::gateway::protocol::GLOBAL) {
        scope = SCOPE_GLOBAL;
      } else {
        scope = SCOPE_LOCAL;
      }
      
      bool result = GatewayCore::getInstance()->unregisterPullInterest(mime_type, scope, this);
      if(result == true) {
        for(std::vector<std::string>::iterator it = registeredPullRequestHandlers.begin(); it != registeredPullRequestHandlers.end(); it++) {
          if((*it) == mime_type) {
            registeredPullRequestHandlers.erase(it);
            break;
          }
        }
      }
      break;
    }
    default: {
      LOG_ERROR("Received unsupported message:" << msg->DebugString());
      break;
    }
  }
  
  delete msg;
  
  return 0;
}

int GatewayEventHandler::onError(const char errorCode) {
  LOG_ERROR("GatewayEventHandler::onError(" << errorCode << ")");
  return 0;
}

bool GatewayEventHandler::sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, MessageScope scope, char priority) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_encoding(encoding);
  pushMsg->set_data(data);
  pushMsg->set_origin_user(originUser);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  msg->set_message_priority(priority);
  
  LOG_DEBUG("Sending Data Push message to connected plugin");
  this->sendMessage(msg);
  
  return true;
}

bool GatewayEventHandler::sendPullRequest(std::string requestUid, std::string pluginId, std::string mimeType, 
                                           std::string query, std::string projection, unsigned int maxResults, 
                                           unsigned int startFromCount, bool liveQuery, char priority) {
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
  
  LOG_DEBUG("Sending Pull Request message to connected plugin");
  this->sendMessage(msg);
  
  
  return true;
}

bool GatewayEventHandler::sendPullResponse(std::string requestUid, std::string pluginId, std::string mimeType,
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
  
  LOG_DEBUG("Sending Pull Response message to connected plugin");
  this->sendMessage(msg);
  
  return true;
}

GatewayEventHandler::~GatewayEventHandler() {
  LOG_DEBUG("GatewayEventHandler being destroyed!");
  LOG_DEBUG("Unregistering data handlers...");
  for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
    GatewayCore::getInstance()->unregisterDataInterest(*it, SCOPE_ALL, this);
  }
  
  LOG_DEBUG("Unregistering pull request handlers...");
  for(std::vector<std::string>::iterator it = registeredPullRequestHandlers.begin(); it != registeredPullRequestHandlers.end(); it++) {
    GatewayCore::getInstance()->unregisterPullInterest(*it, SCOPE_ALL, this);
  }
  
  LOG_DEBUG("Unregistering pull response plugin IDs...");
  for(std::set<std::string>::iterator it = registeredPullResponsePluginIds.begin(); it != registeredPullResponsePluginIds.end(); it++) {
    GatewayCore::getInstance()->unregisterPullResponsePluginId(*it, this);
  }
}
