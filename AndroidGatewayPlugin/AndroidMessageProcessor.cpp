#include "AndroidMessageProcessor.h"

#include "log.h"

AndroidMessageProcessor::AndroidMessageProcessor(AndroidServiceHandler *serviceHandler) :
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler)
{
  
}

AndroidMessageProcessor::~AndroidMessageProcessor() {
  
}

int AndroidMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int AndroidMessageProcessor::close(unsigned long flags) {
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
    newMessageMutex.acquire();
    newMessageAvailable.wait();
    newMessageMutex.release();
    
    ammo::protocol::MessageWrapper *msg = commsHandler->getNextReceivedMessage();
    if(msg) {
      processMessage(*msg);
      delete msg;
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
  LOG_TRACE("Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG("Received Authentication Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
      gatewayConnector->associateDevice(authMessage.device_id(), authMessage.user_id(), authMessage.user_key());
      deviceId = authMessage.device_id();
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG("Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      gatewayConnector->pushData(dataMessage.uri(), dataMessage.mime_type(), dataMessage.data());
      ammo::protocol::MessageWrapper ackMsg;
      ammo::protocol::PushAcknowledgement *ack = ackMsg.mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg.set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG("Sending push acknowledgement to connected device...");
      this->sendData(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG("Received Subscribe Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      gatewayConnector->pullRequest( pullRequest.request_uid(), pullRequest.plugin_id(), pullRequest.mime_type(), pullRequest.query(),
				     pullRequest.projection(), pullRequest.max_results(), pullRequest.start_from_count(), pullRequest.live_query() );

    }
  }
}
