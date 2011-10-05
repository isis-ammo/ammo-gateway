#include "GatewayConnector.h"
#include "GatewayConfigurationManager.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

#include "log.h"

using namespace std;

ammo::gateway::GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : delegate(delegate), handler(NULL), connected(false) {
  init(delegate, ammo::gateway::internal::GatewayConfigurationManager::getInstance());
}

ammo::gateway::GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate, std::string configFile) : delegate(delegate), handler(NULL), connected(false) {
  init(delegate, ammo::gateway::internal::GatewayConfigurationManager::getInstance(configFile.c_str()));
}

void ammo::gateway::GatewayConnector::init(GatewayConnectorDelegate *delegate, ammo::gateway::internal::GatewayConfigurationManager *config) { 
  ACE_INET_Addr serverAddress(config->getGatewayPort(), config->getGatewayAddress().c_str());
  connector = new ACE_Connector<ammo::gateway::internal::GatewayServiceHandler, ACE_SOCK_Connector>();
  int status = connector->connect(handler, serverAddress);
  if(status == -1) {
    LOG_ERROR("connection failed");
    LOG_ERROR("errno: " << errno);
    LOG_ERROR("error: " << strerror(errno));
    connected = false;
  } else {
    connected = true;
    handler->setParentConnector(this);
  }
  if(handler == NULL) {
    LOG_ERROR("Handler not created by ACE_Connector");
  } else {
    LOG_DEBUG("Gateway service handler created by ACE_Connector");
  }
}

ammo::gateway::GatewayConnector::~GatewayConnector() {
  //LOG_DEBUG("Deleting GatewayConnector()");
  if(connected) {
    handler->close();
    connector->close();
  }
  delete connector;
}
  
bool ammo::gateway::GatewayConnector::associateDevice(string device, string user, string key) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::AssociateDevice *associateMsg = msg->mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  LOG_DEBUG("Sending Associate Device message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pushData(ammo::gateway::PushData &pushData) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(pushData.uri);
  pushMsg->set_mime_type(pushData.mimeType);
  pushMsg->set_encoding(pushData.encoding);
  pushMsg->set_data(pushData.data);
  
  if(pushData.scope == SCOPE_LOCAL) {
    pushMsg->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    pushMsg->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pullRequest(PullRequest &request) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PullRequest *pullMsg = msg->mutable_pull_request();
  pullMsg->set_request_uid(request.requestUid);
  pullMsg->set_plugin_id(request.pluginId);
  pullMsg->set_mime_type(request.mimeType);
  pullMsg->set_query(request.query);
  pullMsg->set_projection(request.projection);
  pullMsg->set_max_results(request.maxResults);
  pullMsg->set_start_from_count(request.startFromCount);
  pullMsg->set_live_query(request.liveQuery);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST);
  
  LOG_DEBUG("Sending Pull Request message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pullResponse(PullResponse &response) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PullResponse *pullMsg = msg->mutable_pull_response();
  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_encoding(response.encoding);
  pullMsg->set_data(response.data);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE);
  
  LOG_DEBUG("Sending Pull Response message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::registerDataInterest(string mime_type, DataPushReceiverListener *listener, MessageScope scope) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RegisterDataInterest *di = msg->mutable_register_data_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending RegisterDataInterest message to gateway core");
  if(connected) {
    handler->sendData(msg);
    receiverListeners[mime_type] = listener;
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    LOG_ERROR("Receiver listener was not registered; it won't receive any data.");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::unregisterDataInterest(string mime_type, MessageScope scope) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterDataInterest *di = msg->mutable_unregister_data_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending UnregisterDataInterest message to gateway core");
  if(connected) {
    handler->sendData(msg);
    receiverListeners.erase(mime_type);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::registerPullInterest(string mime_type, PullRequestReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RegisterPullInterest *di = msg->mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending RegisterPullInterest message to gateway core");
  if(connected) {
    handler->sendData(msg);
    pullRequestListeners[mime_type] = listener;
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    LOG_ERROR("Pull Request listener was not registered; it won't receive any data.");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::unregisterPullInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterPullInterest *di = msg->mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending UnregisterPullInterest message to gateway core");
  if(connected) {
    handler->sendData(msg);
    pullRequestListeners.erase(mime_type);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send pull");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::registerPullResponseInterest(string mime_type, PullResponseReceiverListener *listener) {
  pullResponseListeners[mime_type] = listener;
  return true;
}

bool ammo::gateway::GatewayConnector::unregisterPullResponseInterest(string mime_type) {
  pullResponseListeners.erase(mime_type);
  return true;
}


void ammo::gateway::GatewayConnector::onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg) {
  LOG_INFO("Got associate result of " << msg.result());
  if(delegate != NULL) {
    delegate->onAuthenticationResponse(this, msg.result());
  }
}

void ammo::gateway::GatewayConnector::onPushDataReceived(const ammo::gateway::protocol::PushData &msg) {
  ammo::gateway::PushData pushData;
  
  pushData.uri = msg.uri();
  pushData.mimeType = msg.mime_type();
  pushData.encoding = msg.encoding();
  pushData.data.assign(msg.data().begin(), msg.data().end());
  pushData.originUsername = msg.origin_user();
  
  for(map<string, DataPushReceiverListener *>::iterator it = receiverListeners.begin(); it != receiverListeners.end(); it++) {
    if(pushData.mimeType.find(it->first) == 0) {
      it->second->onPushDataReceived(this, pushData);
    }
  }
}

void ammo::gateway::GatewayConnector::onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullRequestReceiverListener *>::iterator it = pullRequestListeners.find(mimeType);
  if ( it != pullRequestListeners.end() ) {
    ammo::gateway::PullRequest req;
    req.requestUid = msg.request_uid();
    req.pluginId = msg.plugin_id();
    req.mimeType = msg.mime_type();
    req.query = msg.query();
    req.projection = msg.projection();
    req.startFromCount = msg.start_from_count();
    req.liveQuery = msg.live_query();
    (*it).second->onPullRequestReceived(this, req);
  } 
}


void ammo::gateway::GatewayConnector::onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg) {
  string mimeType = msg.mime_type();
  for(map<string, PullResponseReceiverListener *>::iterator it = pullResponseListeners.begin(); it != pullResponseListeners.end(); it++) {
    if(mimeType.find(it->first) == 0) {
      PullResponse response;
      response.requestUid = msg.request_uid();
      response.pluginId = msg.plugin_id();
      response.mimeType = msg.mime_type();
      response.uri = msg.uri();
      response.encoding = msg.encoding();
      response.data.assign(msg.data().begin(), msg.data().end());
      (*it).second->onPullResponseReceived(this, response );
    }
  }
}

void ammo::gateway::GatewayConnector::onMultigatewayConnected(const std::string &gatewayId) {
  if(delegate != NULL) {
    delegate->onRemoteGatewayAvailable(this, gatewayId);
  }
}

void ammo::gateway::GatewayConnector::onMultigatewayDisconnected(const std::string &gatewayId) {
  if(delegate != NULL) {
    delegate->onRemoteGatewayUnavailable(this, gatewayId);
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void ammo::gateway::GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  //LOG_INFO("GatewayConnectorDelegate::onAuthenticationResponse : result = " << result);
}

void ammo::gateway::GatewayConnectorDelegate::onRemoteGatewayAvailable(GatewayConnector *sender, std::string gatewayId) {
  //default implementation does nothing
}

void ammo::gateway::GatewayConnectorDelegate::onRemoteGatewayUnavailable(GatewayConnector *sender, std::string gatewayId) {
  //default implementation does nothing
}

//Constructors for PushMessage, PullRequest, PullResponse--  set up sane defaults
ammo::gateway::PushData::PushData() :
  uri(""),
  mimeType(""),
  encoding("json"),
  data(),
  originUsername(""),
  scope(ammo::gateway::SCOPE_GLOBAL)
{
  
}

ammo::gateway::PullRequest::PullRequest() :
  requestUid(""),
  pluginId(""),
  mimeType(""),
  query(""),
  projection(""),
  maxResults(0),
  startFromCount(0),
  liveQuery(false)
{
  
}

ammo::gateway::PullResponse::PullResponse() :
  requestUid(""),
  pluginId(""),
  mimeType(""),
  uri(""),
  encoding("json"),
  data()
{
  
}

ammo::gateway::PullResponse ammo::gateway::PullResponse::createFromPullRequest(ammo::gateway::PullRequest &request) {
  ammo::gateway::PullResponse newResponse;
  newResponse.requestUid = request.requestUid;
  newResponse.pluginId = request.pluginId;
  newResponse.mimeType = request.mimeType;
  return newResponse;
}
