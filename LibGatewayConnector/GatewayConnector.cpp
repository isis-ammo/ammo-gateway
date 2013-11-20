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

#include "GatewayConnector.h"
#include "GatewayConfigurationManager.h"
#include "protocol/GatewayPrivateMessages.pb.h"

#include "GatewayEventHandler.h"

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
  connector = new ammo::gateway::internal::NetworkConnector<ammo::gateway::protocol::GatewayWrapper, ammo::gateway::internal::GatewayEventHandler, ammo::gateway::internal::SYNC_MULTITHREADED, 0xdeadbeef>();
  
  int status = connector->connect(config->getGatewayAddress(), config->getGatewayPort(), handler);
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
    LOG_ERROR("Handler not created by NetworkConnector");
  } else {
    LOG_DEBUG("Gateway event handler created by NetworkConnector");
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
  msg->set_message_priority(ammo::gateway::PRIORITY_AUTH);
  
  LOG_DEBUG("Sending Associate Device message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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
  pushMsg->set_origin_user(pushData.originUsername);
  pushMsg->set_origin_device(pushData.originDevice);
  
  if(pushData.scope == SCOPE_LOCAL) {
    pushMsg->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    pushMsg->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  ammo::gateway::protocol::AcknowledgementThresholds *thresholds = pushMsg->mutable_thresholds();
  thresholds->set_device_delivered(pushData.ackThresholds.deviceDelivered);
  thresholds->set_plugin_delivered(pushData.ackThresholds.pluginDelivered);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA);
  msg->set_message_priority(pushData.priority);
  
  LOG_DEBUG("Sending Data Push message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::pushAcknowledgement(ammo::gateway::PushAcknowledgement &ack) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::PushAcknowledgement *ackMsg = msg->mutable_push_acknowledgement();
  ackMsg->set_uid(ack.uid);
  ackMsg->set_destination_device(ack.destinationDevice);
  ackMsg->set_acknowledging_device(ack.acknowledgingDevice);
  ackMsg->set_destination_user(ack.destinationUser);
  ackMsg->set_acknowledging_user(ack.acknowledgingUser);
  
  ammo::gateway::protocol::AcknowledgementThresholds *thresholds = ackMsg->mutable_threshold();
  thresholds->set_device_delivered(ack.deviceDelivered);
  thresholds->set_plugin_delivered(ack.pluginDelivered);
  
  ammo::gateway::protocol::PushAcknowledgement_PushStatus status = ammo::gateway::protocol::PushAcknowledgement_PushStatus_RECEIVED;
  
  switch(ack.status) {
    case PUSH_RECEIVED:
      status = ammo::gateway::protocol::PushAcknowledgement_PushStatus_RECEIVED;
      break;
    case PUSH_SUCCESS:
      status = ammo::gateway::protocol::PushAcknowledgement_PushStatus_SUCCESS;
      break;
    case PUSH_FAIL:
      status = ammo::gateway::protocol::PushAcknowledgement_PushStatus_FAIL;
      break;
    case PUSH_REJECTED:
      status = ammo::gateway::protocol::PushAcknowledgement_PushStatus_REJECTED;
      break;
  }
  
  ackMsg->set_status(status);
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
  msg->set_message_priority(ammo::gateway::PRIORITY_CTRL); //TODO: is this the right priority for an acknowledgment?
  
  LOG_DEBUG("Sending Push Acknowledgement message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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
  
  if(request.scope == SCOPE_LOCAL) {
    pullMsg->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    pullMsg->set_scope(ammo::gateway::protocol::GLOBAL);
  }

  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST);
  msg->set_message_priority(request.priority);
  
  LOG_DEBUG("Sending Pull Request message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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
  msg->set_message_priority(response.priority);
  
  LOG_DEBUG("Sending Pull Response message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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
  msg->set_message_priority(ammo::gateway::PRIORITY_CTRL);
  
  LOG_DEBUG("Sending RegisterDataInterest message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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
  msg->set_message_priority(ammo::gateway::PRIORITY_CTRL);
  
  LOG_DEBUG("Sending UnregisterDataInterest message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
    receiverListeners.erase(mime_type);
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::registerPullInterest(string mime_type, PullRequestReceiverListener *listener, MessageScope scope) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();;
  ammo::gateway::protocol::RegisterPullInterest *di = msg->mutable_register_pull_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_REGISTER_PULL_INTEREST);
  msg->set_message_priority(ammo::gateway::PRIORITY_CTRL);
  
  LOG_DEBUG("Sending RegisterPullInterest message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
    pullRequestListeners[mime_type] = listener;
    return true;
  } else {
    LOG_ERROR("Not connected to gateway; can't send data");
    LOG_ERROR("Pull Request listener was not registered; it won't receive any data.");
    return false;
  }
}

bool ammo::gateway::GatewayConnector::unregisterPullInterest(string mime_type, MessageScope scope) {
  ammo::gateway::protocol::GatewayWrapper *msg = new ammo::gateway::protocol::GatewayWrapper();
  ammo::gateway::protocol::UnregisterPullInterest *di = msg->mutable_unregister_pull_interest();
  di->set_mime_type(mime_type);
  
  if(scope == SCOPE_LOCAL) {
    di->set_scope(ammo::gateway::protocol::LOCAL);
  } else {
    di->set_scope(ammo::gateway::protocol::GLOBAL);
  }
  
  msg->set_type(ammo::gateway::protocol::GatewayWrapper_MessageType_UNREGISTER_PULL_INTEREST);
  msg->set_message_priority(ammo::gateway::PRIORITY_CTRL);
  
  LOG_DEBUG("Sending UnregisterPullInterest message to gateway core");
  if(connected) {
    handler->sendMessage(msg);
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

void ammo::gateway::GatewayConnector::onConnectReceived() {
  connected = true;
  if(delegate != NULL) {
    delegate->onConnect(this);
  }
}

void ammo::gateway::GatewayConnector::onDisconnectReceived() {
  connected = false;
  if(delegate != NULL) {
    delegate->onDisconnect(this);
  }
}

void ammo::gateway::GatewayConnector::onAssociateResultReceived(const ammo::gateway::protocol::AssociateResult &msg) {
  LOG_INFO("Got associate result of " << msg.result());
  if(delegate != NULL) {
    delegate->onAuthenticationResponse(this, msg.result());
  }
}

void ammo::gateway::GatewayConnector::onPushDataReceived(const ammo::gateway::protocol::PushData &msg, char messagePriority) {
  ammo::gateway::PushData pushData;
  
  pushData.uri = msg.uri();
  pushData.mimeType = msg.mime_type();
  pushData.encoding = msg.encoding();
  pushData.data.assign(msg.data().begin(), msg.data().end());
  pushData.originUsername = msg.origin_user();
  pushData.originDevice = msg.origin_device();
  pushData.priority = messagePriority;
  pushData.ackThresholds.deviceDelivered = msg.thresholds().device_delivered();
  pushData.ackThresholds.pluginDelivered = msg.thresholds().plugin_delivered();
  
  for(map<string, DataPushReceiverListener *>::iterator it = receiverListeners.begin(); it != receiverListeners.end(); it++) {
    if(pushData.mimeType.find(it->first) == 0) {
      it->second->onPushDataReceived(this, pushData);
    }
  }
}

void ammo::gateway::GatewayConnector::onPushAcknowledgementReceived(const ammo::gateway::protocol::PushAcknowledgement &msg) {
  ammo::gateway::PushAcknowledgement pushAck;
  
  pushAck.uid = msg.uid();
  pushAck.destinationDevice = msg.destination_device();
  pushAck.acknowledgingDevice = msg.acknowledging_device();
  pushAck.acknowledgingUser = msg.acknowledging_user();
  pushAck.destinationUser = msg.destination_user();
  pushAck.deviceDelivered = msg.threshold().device_delivered();
  pushAck.pluginDelivered = msg.threshold().plugin_delivered();
  
  switch(msg.status()) {
    case ammo::gateway::protocol::PushAcknowledgement_PushStatus_RECEIVED:
      pushAck.status = ammo::gateway::PUSH_RECEIVED;
      break;
    case ammo::gateway::protocol::PushAcknowledgement_PushStatus_SUCCESS:
      pushAck.status = ammo::gateway::PUSH_SUCCESS;
      break;
    case ammo::gateway::protocol::PushAcknowledgement_PushStatus_FAIL:
      pushAck.status = ammo::gateway::PUSH_FAIL;
      break;
    case ammo::gateway::protocol::PushAcknowledgement_PushStatus_REJECTED:
      pushAck.status = ammo::gateway::PUSH_REJECTED;
      break;
  }
  
  if(delegate != NULL) {
    delegate->onPushAcknowledgementReceived(this, pushAck);
  }
  
}

void ammo::gateway::GatewayConnector::onPullRequestReceived(const ammo::gateway::protocol::PullRequest &msg, char messagePriority) {
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
    req.priority = messagePriority;
    (*it).second->onPullRequestReceived(this, req);
  } 
}


void ammo::gateway::GatewayConnector::onPullResponseReceived(const ammo::gateway::protocol::PullResponse &msg, char messagePriority) {
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
      response.priority = messagePriority;
      (*it).second->onPullResponseReceived(this, response );
    }
  }
}




//--GatewayConnectorDelegate default implementations (for optional delegate methods)

void ammo::gateway::GatewayConnectorDelegate::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  //LOG_INFO("GatewayConnectorDelegate::onAuthenticationResponse : result = " << result);
}

void ammo::gateway::GatewayConnectorDelegate::onPushAcknowledgementReceived(GatewayConnector *sender, const ammo::gateway::PushAcknowledgement &ack) {
  //Do nothing by default
}

//Constructors for PushMessage, PullRequest, PullResponse--  set up sane defaults
ammo::gateway::PushData::PushData() :
  uri(""),
  mimeType(""),
  encoding("json"),
  data(),
  originUsername(""),
  originDevice(""),
  scope(ammo::gateway::SCOPE_GLOBAL),
  priority(ammo::gateway::PRIORITY_NORMAL),
  ackThresholds()
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
  liveQuery(false),
  scope(ammo::gateway::SCOPE_LOCAL),
  priority(ammo::gateway::PRIORITY_NORMAL)
{
  
}

ammo::gateway::PushAcknowledgement::PushAcknowledgement() :
  uid(""),
  destinationDevice(""),
  acknowledgingDevice(""),
  destinationUser(""),
  acknowledgingUser(""),
  deviceDelivered(false),
  pluginDelivered(false),
  status(ammo::gateway::PUSH_RECEIVED)
{
  
}

ammo::gateway::PullResponse::PullResponse() :
  requestUid(""),
  pluginId(""),
  mimeType(""),
  uri(""),
  encoding("json"),
  data(),
  priority(ammo::gateway::PRIORITY_NORMAL)
{
  
}

ammo::gateway::PullResponse ammo::gateway::PullResponse::createFromPullRequest(ammo::gateway::PullRequest &request) {
  ammo::gateway::PullResponse newResponse;
  newResponse.requestUid = request.requestUid;
  newResponse.pluginId = request.pluginId;
  newResponse.mimeType = request.mimeType;
  newResponse.priority = request.priority;
  return newResponse;
}
