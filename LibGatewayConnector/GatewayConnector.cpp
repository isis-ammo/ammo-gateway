#include "GatewayConnector.h"
#include "GatewayConfigurationManager.h"
#include "GatewayConnectionManager.h"
#include "ace/Connector.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <string>
#include <iostream>

#include "log.h"

using namespace std;
using namespace ammo::gateway::internal;

ammo::gateway::GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate) : delegate(delegate), connector(NULL), handler(NULL), connected(false), closing(false), connectionManager(NULL) {
  init(delegate, GatewayConfigurationManager::getInstance());
}

ammo::gateway::GatewayConnector::GatewayConnector(GatewayConnectorDelegate *delegate, std::string configFile) : delegate(delegate), connector(NULL), handler(NULL), connected(false), closing(false), connectionManager(NULL) {
  init(delegate, GatewayConfigurationManager::getInstance(configFile.c_str()));
}

void ammo::gateway::GatewayConnector::init(GatewayConnectorDelegate *delegate, GatewayConfigurationManager *config) { 
  ACE_Reactor::instance(); //we get the singleton reactor here to make sure that
                           //it's been created before the connectionManager thread
                           //starts (you can get sleep_hook error messages if
                           //it isn't, probably as a result of some timing issue)
  connectionManager = new GatewayConnectionManager(this);
  connectionManager->activate();
}

void ammo::gateway::GatewayConnector::onConnect(ACE_Connector<GatewayServiceHandler, ACE_SOCK_Connector> *connector, GatewayServiceHandler *handler) {
  LOG_DEBUG("GatewayConnector got onConnect");
  connected = true;
  this->connector = connector;
  this->handler = handler;
  
  LOG_DEBUG("Restarting reactor to send queued messages");
  
  if(connected && !sendQueue.empty()) {
    handler->reactor()->schedule_wakeup(handler, ACE_Event_Handler::WRITE_MASK);
  }
  
  if(delegate) {
    delegate->onConnect(this);
  }
}

void ammo::gateway::GatewayConnector::onDisconnect() {
  LOG_DEBUG("GatewayConnector got onDisconnect");
  connected = false;
  this->connector = NULL;
  this->handler = NULL;
  if(delegate) {
    delegate->onDisconnect(this);
  }
  if(!closing) {
    LOG_DEBUG("Restarting reconnection loop...");
    connectionManager->activate();
  }
}

ammo::gateway::GatewayConnector::~GatewayConnector() {
  //LOG_DEBUG("Deleting GatewayConnector()");
  closing = true;
  
  if(connected) {
    if(handler != NULL) {
      handler->close();
    } else {
      LOG_WARN("handler was null while deleting GatewayConnector");
    }
    if(connector != NULL) {
      connector->close();
    } else {
      LOG_WARN("connector was null while deleting GatewayConnector");
    }
  }
  if(handler) {
    delete handler;
    handler = 0;
  }
  if(connector) {
    delete connector;
    connector = 0;
  }
  connectionManager->cancel();
  connectionManager->wait();
  delete connectionManager;
}

void ammo::gateway::GatewayConnector::sendMessage(ammo::gateway::protocol::GatewayWrapper *msg) {
  sendQueueMutex.acquire();
  sendQueue.push(msg);
  LOG_TRACE("Queued a message to send.  " << sendQueue.size() << " messages in queue.");
  sendQueueMutex.release();
  
  if(connected) { //TODO: is this thread-safe?  Maybe not (depends on where connected gets changed and where sendMessage gets called)
    handler->reactor()->schedule_wakeup(handler, ACE_Event_Handler::WRITE_MASK);
  }
}

/* Gets the next message to be sent (but doesn't remove it from the queue).  It's
   removed from the queue after it's been completely sent (so that we can resend
   the message if our connection is disrupted during send).
*/
ammo::gateway::protocol::GatewayWrapper *ammo::gateway::GatewayConnector::getNextMessageToSend() {
  ammo::gateway::protocol::GatewayWrapper *msg = NULL;
  
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    msg = sendQueue.front();
  }
  sendQueueMutex.release();
  
  LOG_TRACE("Got a message to send.");
  return msg;
}

void ammo::gateway::GatewayConnector::removeSentMessageFromQueue() {  
  sendQueueMutex.acquire();
  if(!sendQueue.empty()) {
    sendQueue.pop();
  } else {
    LOG_WARN("Send queue is empty...  send queue shouldn't be empty when this method is called.");
  }
  
  int size = sendQueue.size();
  sendQueueMutex.release();
  
  LOG_TRACE("Finished sending and removed a message from queue.  " << size << " messages remain in queue.");
}
  
bool ammo::gateway::GatewayConnector::associateDevice(string device, string user, string key) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::AssociateDevice *associateMsg = msg->mutable_associate_device();
  associateMsg->set_device(device);
  associateMsg->set_user(user);
  associateMsg->set_key(key);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_DEVICE);
  
  LOG_DEBUG("Sending Associate Device message to gateway core");
  sendMessage(msg);
  return true;
}

bool ammo::gateway::GatewayConnector::pushData(ammo::gateway::PushData &pushData) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushData *pushMsg = msg->mutable_push_data();
  pushMsg->set_uri(pushData.uri);
  pushMsg->set_mime_type(pushData.mimeType);
  pushMsg->set_data(pushData.data);
  
  if(pushData.scope == SCOPE_LOCAL) {
    pushMsg->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    pushMsg->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  
  LOG_DEBUG("Sending Data Push message to gateway core");
  sendMessage(msg);
  return true;
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
  sendMessage(msg);
  return true;
}

bool ammo::gateway::GatewayConnector::pullResponse(PullResponse &response) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PullResponse *pullMsg = msg->mutable_pull_response();
  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_data(response.data);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE);
  
  LOG_DEBUG("Sending Pull Response message to gateway core");
  sendMessage(msg);
  return true;
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
  sendMessage(msg);
  receiverListeners[mime_type] = listener;
  return true;
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
  sendMessage(msg);
  receiverListeners.erase(mime_type);
  return true;
}

bool ammo::gateway::GatewayConnector::registerPullInterest(string mime_type, PullRequestReceiverListener *listener) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::RegisterPullInterest *di = msg->mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending RegisterPullInterest message to gateway core");
  sendMessage(msg);
  pullRequestListeners[mime_type] = listener;
  return true;
}

bool ammo::gateway::GatewayConnector::unregisterPullInterest(string mime_type) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterPullInterest *di = msg->mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  
  LOG_DEBUG("Sending UnregisterPullInterest message to gateway core");
  sendMessage(msg);
  pullRequestListeners.erase(mime_type);
  return true;
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
      response.data.assign(msg.data().begin(), msg.data().end());
      (*it).second->onPullResponseReceived(this, response );
    }
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void ammo::gateway::GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  //LOG_INFO("GatewayConnectorDelegate::onAuthenticationResponse : result = " << result);
}

//Constructors for PushMessage, PullRequest, PullResponse--  set up sane defaults
ammo::gateway::PushData::PushData() :
  uri(""),
  mimeType(""),
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
