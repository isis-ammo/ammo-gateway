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
    GatewayCore::getInstance()->pushCrossGateway(msg->push_data().uri(), msg->push_data().mime_type(), msg->push_data().encoding(), msg->push_data().data(), msg->push_data().origin_user(), gatewayId);
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
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendUnsubscribeMessage(std::string mime_type) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterDataInterest *unsubscribeMsg = msg->mutable_unregister_data_interest();
  unsubscribeMsg->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending Subscribe message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

bool CrossGatewayEventHandler::sendPushedData(std::string uri, std::string mimeType, std::string encoding, const std::string &data, std::string originUser) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  pushMsg->set_encoding(encoding);
  pushMsg->set_origin_user(originUser);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to connected gateway");
  this->sendMessage(msg);
  
  return true;
}

CrossGatewayEventHandler::~CrossGatewayEventHandler() {
  LOG_DEBUG("CrossGatewayEventHandler being destroyed!");
  
  LOG_DEBUG("Unregistering data handlers...");
  for(std::vector<std::string>::iterator it = registeredHandlers.begin(); it != registeredHandlers.end(); it++) {
    GatewayCore::getInstance()->unsubscribeCrossGateway(*it, gatewayId);
  }
  
  /*LOG_DEBUG("Unregistering pull request handlers...");
  for(std::vector<std::string>::iterator it = registeredPullRequestHandlers.begin(); it != registeredPullRequestHandlers.end(); it++) {
    //GatewayCore::getInstance()->unregisterPullInterest(*it, this);
  }*/
  
  if(registeredWithGateway) {
    GatewayCore::getInstance()->unregisterCrossGatewayConnection(gatewayId);
  }
}
