#include "AndroidMessageProcessor.h"
#include "AndroidServiceHandler.h"

#include "log.h"

using namespace ammo::gateway;

const char DEFAULT_PRIORITY = 50;

AndroidMessageProcessor::AndroidMessageProcessor(AndroidServiceHandler *serviceHandler) :
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
      pushData.uri = dataMessage.uri();
      pushData.mimeType = dataMessage.mime_type();
      pushData.data = dataMessage.data();
      pushData.scope = scope;
      gatewayConnector->pushData(pushData);
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG(commsHandler << " Sending push acknowledgment to connected device...");
      ackMsg->set_message_priority(DEFAULT_PRIORITY);
      commsHandler->sendMessage(ackMsg, DEFAULT_PRIORITY);
      
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
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DIRECTED_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Directed message");
    if(gatewayConnector != NULL) {
      ammo::protocol::DirectedMessage dirMsg = msg.directed_message();
      ammo::gateway::DirectedMessage message;
      message.uri = dirMsg.uri();
      message.destinationUser = dirMsg.destination_user();
      message.mimeType = dirMsg.mime_type();
      message.data = dirMsg.data();
      message.originUser = dirMsg.origin_user();
      
      if(dirMsg.scope() == ammo::protocol::LOCAL) {
        message.scope = SCOPE_LOCAL;
      } else {
        message.scope = SCOPE_GLOBAL;
      }
      
      gatewayConnector->directedMessage(message);
    } 
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG((long) commsHandler << " Received Pull Request Message...");
    if(gatewayConnector != NULL && deviceIdAuthenticated) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
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
    heartbeatAck->set_message_priority(DEFAULT_PRIORITY);
    
    LOG_DEBUG((long) commsHandler << " Sending heartbeat acknowledgement to connected device...");
    commsHandler->sendMessage(heartbeatAck, DEFAULT_PRIORITY);
  }
}

void AndroidMessageProcessor::onConnect(GatewayConnector *sender) {
}

void AndroidMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidMessageProcessor::onDirectedMessage(GatewayConnector *sender, ammo::gateway::DirectedMessage &directedMsg) {
  LOG_DEBUG(commsHandler << " Sending directed message to device...");
  LOG_DEBUG(commsHandler << "    " << directedMsg);
  
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DirectedMessage *dirMsg = msg->mutable_directed_message();
  dirMsg->set_uri(directedMsg.uri);
  dirMsg->set_destination_user(directedMsg.destinationUser);
  dirMsg->set_mime_type(directedMsg.mimeType);
  dirMsg->set_data(directedMsg.data);
  dirMsg->set_origin_user(directedMsg.originUser);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DIRECTED_MESSAGE);
  msg->set_message_priority(DEFAULT_PRIORITY);
  
  LOG_DEBUG(commsHandler << " Sending Directed Message to connected device");
  commsHandler->sendMessage(msg, DEFAULT_PRIORITY);
}

void AndroidMessageProcessor::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  LOG_DEBUG((long) commsHandler << " Sending subscribed data to device...");
  LOG_DEBUG((long) commsHandler << "    " << pushData);
  
  std::string dataString(pushData.data.begin(), pushData.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uri(pushData.uri);
  dataMsg->set_mime_type(pushData.mimeType);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  msg->set_message_priority(DEFAULT_PRIORITY);
  
  LOG_DEBUG((long) commsHandler << " Sending Data Push message to connected device");
  commsHandler->sendMessage(msg, DEFAULT_PRIORITY);
}

void AndroidMessageProcessor::onPullResponseReceived(GatewayConnector *sender, ammo::gateway::PullResponse &response) {
  LOG_DEBUG((long) commsHandler << " Sending pull response to device...");
  LOG_DEBUG((long) commsHandler << "    URI: " << response.uri << ", Type: " << response.mimeType);
  
  std::string dataString(response.data.begin(), response.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  msg->set_message_priority(DEFAULT_PRIORITY);
  
  LOG_DEBUG((long) commsHandler << " Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg, DEFAULT_PRIORITY);
}



void AndroidMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  LOG_DEBUG((long) commsHandler << " Delegate: onAuthenticationResponse");
  if(result == true) {
    deviceIdAuthenticated = true;
  }
  
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg->set_message_priority(DEFAULT_PRIORITY);
  newMsg->mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  commsHandler->sendMessage(newMsg, DEFAULT_PRIORITY);
}
