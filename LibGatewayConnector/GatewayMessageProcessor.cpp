#include "GatewayMessageProcessor.h"
#include "GatewayServiceHandler.h"

#include "log.h"

GatewayMessageProcessor::GatewayMessageProcessor(GatewayServiceHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler)
{
}

GatewayMessageProcessor::~GatewayMessageProcessor() {
  LOG_TRACE("In ~GatewayMessageProcessor()");
}

void GatewayMessageProcessor::setParentConnector(GatewayConnector *parent) {
  this->parent = parent;
}

int GatewayMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int GatewayMessageProcessor::close() {
  LOG_TRACE("Closing MessageProcessor (in GatewayMessageProcessor.close())");
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
  
  signalNewMessageAvailable();
  return 0;
}

bool GatewayMessageProcessor::isClosed() {
  volatile bool ret; //does this need to be volatile to keep the compiler from optimizing it out?

  closeMutex.acquire();
  ret = closed;
  closeMutex.release();
  
  return ret;
}

int GatewayMessageProcessor::svc() {
  while(!isClosed()) {
    newMessageMutex.acquire();
    newMessageAvailable.wait();
    newMessageMutex.release();
    
    ammo::gateway::protocol::GatewayWrapper *msg = NULL;
    
    do {
      msg = commsHandler->getNextReceivedMessage();
      if(msg) {
        processMessage(*msg);
        delete msg;
      }
    } while (msg != NULL);
  }
  return 0;
}

void GatewayMessageProcessor::signalNewMessageAvailable() {
  newMessageMutex.acquire();
  newMessageAvailable.signal();
  newMessageMutex.release();
}

void GatewayMessageProcessor::processMessage(ammo::gateway::protocol::GatewayWrapper &msg) {
  LOG_TRACE("Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_ASSOCIATE_RESULT) {
    LOG_DEBUG("Received Associate Result...");
    parent->onAssociateResultReceived(msg.associate_result());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PUSH_DATA) {
    LOG_DEBUG("Received Push Data...");
    parent->onPushDataReceived(msg.push_data());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG("Received Pull Request...");
    parent->onPullRequestReceived(msg.pull_request());
  } else if(msg.type() == ammo::gateway::protocol::GatewayWrapper_MessageType_PULL_RESPONSE) {
    LOG_DEBUG("Received Pull Response...");
    parent->onPullResponseReceived(msg.pull_response());
  }
}

