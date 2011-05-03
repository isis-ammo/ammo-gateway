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
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::AssociateDevice *associateMsg = msg.mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  LOG_DEBUG("Sending Associate Device message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pushData(string uri, string mimeType, const string &data, MessageScope scope) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  
  if(scope == SCOPE_LOCAL) {
    pushMsg->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    pushMsg->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pullRequest(std::string requestUid, std::string pluginId,
				   std::string mimeType, std::string query,
				   std::string projection, unsigned int maxResults,
				   unsigned int startFromCount, bool liveQuery) {

  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PullRequest *pullMsg = msg.mutable_pull_request();
  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_query(query);
  pullMsg->set_projection(projection);
  pullMsg->set_max_results(maxResults);
  pullMsg->set_start_from_count(startFromCount);
  pullMsg->set_live_query(liveQuery);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST);
  
  LOG_DEBUG("Sending Pull Request message to gateway core");
  if(connected) {
    handler->sendData(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pullResponse(std::string requestUid, std::string pluginId,
				   std::string mimeType, std::string uri,
				    std::vector<char>& data) {

  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PullResponse *pullMsg = msg.mutable_pull_response();
  pullMsg->set_request_uid(requestUid);
  pullMsg->set_plugin_id(pluginId);
  pullMsg->set_mime_type(mimeType);
  pullMsg->set_uri(uri);
  pullMsg->set_data( std::string(data.begin(), data.end()) );
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE);
  
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
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterDataInterest *di = msg.mutable_register_data_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
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
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterDataInterest *di = msg.mutable_unregister_data_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
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
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterPullInterest *di = msg.mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  
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
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterPullInterest *di = msg.mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  
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
  string uri = msg.uri();
  string mimeType = msg.mime_type();
  vector<char> data(msg.data().begin(), msg.data().end());
  string originUser = msg.origin_user();
  
  for(map<string, DataPushReceiverListener *>::iterator it = receiverListeners.begin(); it != receiverListeners.end(); it++) {
    if(mimeType.find(it->first) == 0) {
      it->second->onDataReceived(this, uri, mimeType, data, originUser);
    }
  }
}

void ammo::gateway::GatewayConnector::onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullRequestReceiverListener *>::iterator it = pullRequestListeners.find(mimeType);
  if ( it != pullRequestListeners.end() ) {
    (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.query(),
				 msg.projection(), msg.max_results(), msg.start_from_count(), msg.live_query() );
  } 
}


void ammo::gateway::GatewayConnector::onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg) {
  string mimeType = msg.mime_type();
  for(map<string, PullResponseReceiverListener *>::iterator it = pullResponseListeners.begin(); it != pullResponseListeners.end(); it++) {
    if(mimeType.find(it->first) == 0) {
      vector<char> data(msg.data().begin(), msg.data().end());
      (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.uri(), data );
    }
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void ammo::gateway::GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  //LOG_INFO("GatewayConnectorDelegate::onAuthenticationResponse : result = " << result);
}


