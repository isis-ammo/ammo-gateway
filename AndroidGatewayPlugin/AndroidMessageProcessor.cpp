#include "AndroidMessageProcessor.h"
#include "AndroidEventHandler.h"

#include "log.h"

using namespace ammo::gateway;

const char DEFAULT_PRIORITY = 50;

AndroidMessageProcessor::AndroidMessageProcessor(AndroidEventHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler),
gatewayConnector(NULL),
deviceId(""),
deviceIdAuthenticated(false)
{
  //need to initialize GatewayConnector in the main thread; the constructor always
  //happens in the main thread
  gatewayConnector = new GatewayConnector(this);
}

AndroidMessageProcessor::~AndroidMessageProcessor() {
  LOG_TRACE((long) commsHandler << " In ~AndroidMessageProcessor()");
  if(gatewayConnector) {
    delete gatewayConnector;
  }
}

int AndroidMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int AndroidMessageProcessor::close(unsigned long flags) {
  LOG_TRACE((long) commsHandler << " Closing MessageProcessor (in AndroidMessageProcessor.close())");
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
  
  signalNewMessageAvailable();
  return 0;
}

bool AndroidMessageProcessor::isClosed() {
  volatile bool ret; //does this need to be volatile to keep the compiler from optimizing it out?

  closeMutex.acquire();
  ret = closed;
  closeMutex.release();
  
  return ret;
}

int AndroidMessageProcessor::svc() {
  while(!isClosed()) {    
    ammo::protocol::MessageWrapper *msg = NULL;
    
    do {
      msg = commsHandler->getNextReceivedMessage();
      if(msg) {
        processMessage(*msg);
        delete msg;
      }
    } while (msg != NULL);
    if(!isClosed()) {
      newMessageMutex.acquire();
      newMessageAvailable.wait();
      newMessageMutex.release();
    }
  }
  return 0;
}

void AndroidMessageProcessor::signalNewMessageAvailable() {
  newMessageMutex.acquire();
  newMessageAvailable.signal();
  newMessageMutex.release();
}

void AndroidMessageProcessor::processMessage(ammo::protocol::MessageWrapper &msg) {
  LOG_TRACE((long) commsHandler << " Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Authentication Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
      this->deviceId = authMessage.device_id();
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      MessageScope scope;
      if(dataMessage.scope() == ammo::protocol::LOCAL) {
        scope = SCOPE_LOCAL;
      } else {
        scope = SCOPE_GLOBAL;
      }
      
      PushData pushData;
      pushData.uid = dataMessage.uid();
      pushData.mimeType = dataMessage.mime_type();
      pushData.data = dataMessage.data();
      pushData.scope = scope;
      pushData.encoding = dataMessage.encoding();
      pushData.originUsername = dataMessage.user_id();
      pushData.priority = msg.message_priority();
      gatewayConnector->pushData(pushData);
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uid(dataMessage.uid());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG(commsHandler << " Sending push acknowledgment to connected device...");
      ackMsg->set_message_priority(pushData.priority);
      commsHandler->sendMessage(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Subscribe Message...");
    MessageScope scope;
    if(msg.subscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this, scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_UNSUBSCRIBE_MESSAGE) {
    LOG_DEBUG((long) commsHandler << " Received Unubscribe Message...");
    MessageScope scope;
    if(msg.unsubscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::UnsubscribeMessage unsubscribeMessage = msg.unsubscribe_message();
      
      gatewayConnector->unregisterDataInterest(unsubscribeMessage.mime_type(), scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG((long) commsHandler << " Received Pull Request Message...");
    if(gatewayConnector != NULL && deviceIdAuthenticated) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      
      MessageScope scope;
      if(pullRequest.scope() == ammo::protocol::LOCAL) {
        scope = SCOPE_LOCAL;
      } else {
        scope = SCOPE_GLOBAL;
      }
      // now send request
      PullRequest req;
      req.requestUid = pullRequest.request_uid();
      req.pluginId = this->deviceId;
      req.mimeType = pullRequest.mime_type();
      req.query = pullRequest.query();
      req.projection = pullRequest.projection();
      req.maxResults = pullRequest.max_results();
      req.startFromCount = pullRequest.start_from_count();
      req.liveQuery = pullRequest.live_query();
      req.scope = scope;
      req.priority = msg.message_priority();
      gatewayConnector->pullRequest(req);
    } else {
      if(!deviceIdAuthenticated) {
        LOG_ERROR((long) commsHandler << " Attempted to send a pull request before authentication.");
      }
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_HEARTBEAT) {
    LOG_DEBUG((long) commsHandler << " Received Heartbeat from device...");
    ammo::protocol::Heartbeat heartbeat = msg.heartbeat();
    
    ammo::protocol::MessageWrapper *heartbeatAck = new ammo::protocol::MessageWrapper();
    ammo::protocol::Heartbeat *ack = heartbeatAck->mutable_heartbeat();
    ack->set_sequence_number(heartbeat.sequence_number());
    heartbeatAck->set_type(ammo::protocol::MessageWrapper_MessageType_HEARTBEAT);
    heartbeatAck->set_message_priority(ammo::gateway::PRIORITY_CTRL);
    
    LOG_DEBUG((long) commsHandler << " Sending heartbeat acknowledgement to connected device...");
    commsHandler->sendMessage(heartbeatAck);
  }
}

void AndroidMessageProcessor::onConnect(GatewayConnector *sender) {
}

void AndroidMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidMessageProcessor::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  LOG_DEBUG((long) commsHandler << " Sending subscribed data to device...");
  LOG_DEBUG((long) commsHandler << "    " << pushData);
  
  std::string dataString(pushData.data.begin(), pushData.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uid(pushData.uid);
  dataMsg->set_mime_type(pushData.mimeType);
  dataMsg->set_encoding(pushData.encoding);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  msg->set_message_priority(pushData.priority);
  
  LOG_DEBUG((long) commsHandler << " Sending Data Push message to connected device");
  commsHandler->sendMessage(msg);
}

void AndroidMessageProcessor::onPullResponseReceived(GatewayConnector *sender, ammo::gateway::PullResponse &response) {
  LOG_DEBUG((long) commsHandler << " Sending pull response to device...");
  LOG_DEBUG((long) commsHandler << "    UID: " << response.uid << ", Type: " << response.mimeType);
  
  std::string dataString(response.data.begin(), response.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uid(response.uid);
  pullMsg->set_encoding(response.encoding);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  msg->set_message_priority(response.priority);
  
  LOG_DEBUG((long) commsHandler << " Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg);
}



void AndroidMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  LOG_DEBUG((long) commsHandler << " Delegate: onAuthenticationResponse");
  if(result == true) {
    deviceIdAuthenticated = true;
  }
  
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg->set_message_priority(ammo::gateway::PRIORITY_AUTH);
  newMsg->mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  commsHandler->sendMessage(newMsg);
}
