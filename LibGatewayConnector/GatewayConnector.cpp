#include "GatewayConnector.h"
#include "GatewayConfigurationManager.h"
#include "GatewayConnectionManager.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

#include "log.h"

using namespace std;

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : delegate(delegate), handler(NULL), connected(false), connectionManager(NULL) {
  init(delegate, GatewayConfigurationManager::getInstance());
}

GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate, std::string configFile) : delegate(delegate), handler(NULL), connected(false), connectionManager(NULL) {
  init(delegate, GatewayConfigurationManager::getInstance(configFile.c_str()));
}

void GatewayConnector::init(GatewayConnectorDelegate *delegate, GatewayConfigurationManager *config) { 
  ACE_Reactor::instance(); //we get the singleton reactor here to make sure that
                           //it's been created before the connectionManager thread
                           //starts (you can get sleep_hook error messages if
                           //it isn't, probably as a result of some timing issue)
  connectionManager = new GatewayConnectionManager(this);
  connectionManager->activate();
}

void GatewayConnector::onConnect(ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector> *connector, GatewayServiceHandler *handler) {
  LOG_DEBUG("GatewayConnector got onConnect");
  connected = true;
  this->connector = connector;
  this->handler = handler;
  
  LOG_DEBUG("Sending queued messages");
  
  while(connected && !messageQueue.empty()) {
    ammo::gateway::protocol::GatewayWrapper msg = messageQueue.front();
    handler->sendData(msg);
    messageQueue.pop();
  }
  
  if(delegate) {
    delegate->onConnect(this);
  }
}

void GatewayConnector::onDisconnect() {
  LOG_DEBUG("GatewayConnector got onDisconnect");
  connected = false;
  this->connector = NULL;
  this->handler = NULL;
  if(delegate) {
    delegate->onDisconnect(this);
  }
  LOG_DEBUG("Restarting reconnection loop...");
  connectionManager->activate();
}

GatewayConnector::~GatewayConnector() {
  //LOG_DEBUG("Deleting GatewayConnector()");
  if(connected) {
    if(handler != NULL) {
      handler->close();
    } else {
      LOG_WARN("handler was null while deleting GatewayConnector");
    }
    if(handler != NULL) {
      connector->close();
    } else {
      LOG_WARN("connector was null while deleting GatewayConnector");
    }
  }
  if(handler) {
    delete connector;
  }
}

void GatewayConnector::sendMessage(ammo::gateway::protocol::GatewayWrapper &msg) {
  if(connected) {
    handler->sendData(msg);
  } else {
    LOG_DEBUG("No connection...  queueing message");
    messageQueue.push(msg);
  }
}
  
bool GatewayConnector::associateDevice(string device, string user, string key) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::AssociateDevice *associateMsg = msg.mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  LOG_DEBUG("Sending Associate Device message to gateway core");
  sendMessage(msg);
  return true;
}

bool GatewayConnector::pushData(string uri, string mimeType, const string &data) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::PushData *pushMsg = msg.mutable_push_data();
  pushMsg->set_uri(uri);
  pushMsg->set_mime_type(mimeType);
  pushMsg->set_data(data);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to gateway core");
  sendMessage(msg);
  return true;
}

bool GatewayConnector::pullRequest(std::string requestUid, std::string pluginId,
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
  sendMessage(msg);
  return true;
}

bool GatewayConnector::pullResponse(std::string requestUid, std::string pluginId,
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
  sendMessage(msg);
  return true;
}




bool GatewayConnector::registerDataInterest(string mime_type, DataPushReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterDataInterest *di = msg.mutable_register_data_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending RegisterDataInterest message to gateway core");
  sendMessage(msg);
  receiverListeners[mime_type] = listener;
  return true;
}

bool GatewayConnector::unregisterDataInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterDataInterest *di = msg.mutable_unregister_data_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_DATA_INTEREST);
  
  LOG_DEBUG("Sending UnregisterDataInterest message to gateway core");
  sendMessage(msg);
  receiverListeners.erase(mime_type);
  return true;
}


bool GatewayConnector::registerPullInterest(string mime_type, PullRequestReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::RegisterPullInterest *di = msg.mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending RegisterPullInterest message to gateway core");
  sendMessage(msg);
  pullRequestListeners[mime_type] = listener;
  return true;
}

bool GatewayConnector::unregisterPullInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper msg;
  ammo::gateway::protocol::UnregisterPullInterest *di = msg.mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  msg.set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending UnregisterPullInterest message to gateway core");
  sendMessage(msg);
  pullRequestListeners.erase(mime_type);
  return true;
}

bool GatewayConnector::registerPullResponseInterest(string mime_type, PullResponseReceiverListener *listener) {
  pullResponseListeners[mime_type] = listener;
  return true;
}

bool GatewayConnector::unregisterPullResponseInterest(string mime_type) {
  pullResponseListeners.erase(mime_type);
  return true;
}


void GatewayConnector::onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg) {
  LOG_INFO("Got associate result of " << msg.result());
  if(delegate != NULL) {
    delegate->onAuthenticationResponse(this, msg.result());
  }
}

void GatewayConnector::onPushDataReceived(const ammo::gateway::protocol::PushData &msg) {
  string uri = msg.uri();
  string mimeType = msg.mime_type();
  vector<char> data(msg.data().begin(), msg.data().end());
  string originUser = msg.origin_user();
  
  receiverListeners[mimeType]->onDataReceived(this, uri, mimeType, data, originUser);
}

void GatewayConnector::onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullRequestReceiverListener *>::iterator it = pullRequestListeners.find(mimeType);
  if ( it != pullRequestListeners.end() ) {
    (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.query(),
				 msg.projection(), msg.max_results(), msg.start_from_count(), msg.live_query() );
  } 
}


void GatewayConnector::onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg) {
  string mimeType = msg.mime_type();
  map<std::string, PullResponseReceiverListener *>::iterator it = pullResponseListeners.find(mimeType);
  if ( it != pullResponseListeners.end() ) {
    vector<char> data(msg.data().begin(), msg.data().end());
    (*it).second->onDataReceived(this, msg.request_uid(), msg.plugin_id(), msg.mime_type(), msg.uri(), data );
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  //LOG_INFO("GatewayConnectorDelegate::onAuthenticationResponse : result = " << result);
}


