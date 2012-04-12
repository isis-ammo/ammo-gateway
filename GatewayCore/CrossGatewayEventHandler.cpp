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
  
  std::vector<std::pair<std::string, std::string> > connectedPlugins = GatewayCore::getInstance()->getLocalPlugins();
  
  for(std::vector<std::pair<std::string, std::string> >::iterator it = connectedPlugins.begin(); it != connectedPlugins.end(); ++it) {
    ammo::gateway::protocol::AssociateCrossGateway::PluginInstanceId *plugin = newMsg->mutable_associate_cross_gateway()->add_connected_plugins();
    plugin->set_plugin_name(it->first);
    plugin->set_instance_id(it->second);
  }
  
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
    
    //deserialize list of connected plugins
    std::vector<std::pair<std::string, std::string> > connectedPlugins;
    
	//RepeatedPtrField has no assignment operator or copy constructor under Protobuf 2.3.0 on Windows,
	//so we have to call connected_plugins() each time we want to access the field
    for(google::protobuf::RepeatedPtrField<const ammo::gateway::protocol::AssociateCrossGateway::PluginInstanceId>::iterator it = msg->associate_cross_gateway().connected_plugins().begin(); it != msg->associate_cross_gateway().connected_plugins().end(); ++it) {
      connectedPlugins.push_back(std::make_pair(it->plugin_name(), it->instance_id()));
    }
    
    GatewayCore::getInstance()->registerCrossGatewayConnection(gatewayId, this, connectedPlugins);
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
    GatewayCore::getInstance()->pushCrossGateway(msg->push_data().uri(), msg->push_data().mime_type(), msg->push_data().encoding(), msg->push_data().data(), msg->push_data().origin_user(), gatewayId, msg->message_priority());
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
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_POINT_TO_POINT_MESSAGE) {
    LOG_DEBUG("Received Point to Point Message...");
    ammo::gateway::protocol::PointToPointMessage ptp = msg->point_to_point_message();
    
    GatewayCore::getInstance()->pointToPointMessageCrossGateway(gatewayId, ptp.uid(), ptp.destination_gateway(), ptp.destination_plugin_name(), ptp.destination_instance_id(), ptp.source_gateway(), ptp.source_plugin_name(), ptp.source_instance_id(), ptp.mime_type(), ptp.encoding(), ptp.data(), msg->message_priority());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PLUGIN_CONNECTED_NOTIFICATION) {
    LOG_DEBUG("Received Plugin Connected notification...");
    ammo::gateway::protocol::PluginConnectedNotification ptp = msg->plugin_connected_notification();
    
    GatewayCore::getInstance()->pluginConnectedCrossGateway(ptp.gateway_id(), gatewayId, ptp.plugin_name(), ptp.instance_id());
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_REMOTE_GATEWAY_CONNECTED_NOTIFICATION) {
    LOG_DEBUG("Received Remote Gateway Connected notification...");
    ammo::gateway::protocol::RemoteGatewayConnectedNotification notif = msg->remote_gateway_connected_notification();
    
    std::vector<std::pair<std::string, std::string> > connectedPlugins;
    
    for(google::protobuf::RepeatedPtrField<const ammo::gateway::protocol::RemoteGatewayConnectedNotification::PluginInstanceId>::iterator it = notif.connected_plugins().begin(); it != notif.connected_plugins().end(); ++it) {
      connectedPlugins.push_back(std::make_pair(it->plugin_name(), it->instance_id()));
    }
    
    GatewayCore::getInstance()->gatewayConnectedCrossGateway(notif.gateway_id(), gatewayId, connectedPlugins);
  } else if(msg->type() == ammo::gateway::protocol::GatewayWrapper_MessageType_REMOTE_GATEWAY_DISCONNECTED_NOTIFICATION) {
    LOG_DEBUG("Received Remote Gateway Disconnected notification...");
    ammo::gateway::protocol::RemoteGatewayDisconnectedNotification notif = msg->remote_gateway_disconnected_notification();
    
    GatewayCore::getInstance()->gatewayDisconnectedCrossGateway(notif.gateway_id(), gatewayId);
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

bool CrossGatewayEventHandler::sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser, char priority) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  pushMsg->set_encoding(encoding);
  pushMsg->set_origin_user(originUser);
  
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

bool CrossGatewayEventHandler::sendPointToPointMessage(std::string uid, std::string destinationGateway, std::string destinationPluginName, std::string destinationInstanceId, 
std::string sourceGateway, std::string sourcePluginName, std::string sourceInstanceId, std::string mimeType, std::string encoding, std::string data, char priority) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PointToPointMessage *ptp = msg->mutable_point_to_point_message();
  ptp->set_uid(uid);
  ptp->set_destination_gateway(destinationGateway);
  ptp->set_destination_plugin_name(destinationPluginName);
  ptp->set_destination_instance_id(destinationInstanceId);
  ptp->set_source_gateway(sourceGateway);
  ptp->set_source_plugin_name(sourcePluginName);
  ptp->set_source_instance_id(sourceInstanceId);
  ptp->set_mime_type(mimeType);
  ptp->set_encoding(encoding);
  ptp->set_data(data);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_POINT_TO_POINT_MESSAGE);
  msg->set_message_priority(priority);
  
  LOG_DEBUG("Sending Point to Point message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendRemoteGatewayConnectedNotification(std::string gatewayId, std::vector<std::pair<std::string, std::string> > connectedPlugins) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RemoteGatewayConnectedNotification *notif = msg->mutable_remote_gateway_connected_notification();
  
  notif->set_gateway_id(gatewayId);
  
  for(std::vector<std::pair<std::string, std::string> >::iterator it = connectedPlugins.begin(); it != connectedPlugins.end(); ++it) {
    ammo::gateway::protocol::RemoteGatewayConnectedNotification::PluginInstanceId *plugin = notif->add_connected_plugins();
    plugin->set_plugin_name(it->first);
    plugin->set_instance_id(it->second);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REMOTE_GATEWAY_CONNECTED_NOTIFICATION);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Remote Gateway Connected message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendRemoteGatewayDisconnectedNotification(std::string gatewayId) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RemoteGatewayDisconnectedNotification *notif = msg->mutable_remote_gateway_disconnected_notification();
  
  notif->set_gateway_id(gatewayId);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REMOTE_GATEWAY_DISCONNECTED_NOTIFICATION);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Remote Gateway Disconnected message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendPluginConnectedNotification(std::string pluginName, std::string instanceId, bool remotePlugin, std::string gatewayId) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PluginConnectedNotification *notif = msg->mutable_plugin_connected_notification();
  
  notif->set_plugin_name(pluginName);
  notif->set_instance_id(instanceId);
  notif->set_remote_plugin(remotePlugin);
  notif->set_gateway_id(gatewayId);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PLUGIN_CONNECTED_NOTIFICATION);
  msg->set_message_priority(PRIORITY_CTRL);
  
  LOG_DEBUG("Sending Plugin Connected message to connected gateway");
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
