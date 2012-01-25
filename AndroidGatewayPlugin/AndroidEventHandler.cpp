#include <string>

#include "AndroidEventHandler.h"
#include "NetworkServiceHandler.h"
#include "log.h"

#include "protocol/AmmoMessages.pb.h"
#include "AndroidMessageProcessor.h"

using namespace std;

AndroidEventHandler::AndroidEventHandler() :
messageProcessor(NULL),
receiveQueueMutex(),
receivedMessageCount(0) {
  LOG_TRACE("ctor AndroidEventHandler()");
}

void AndroidEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE("onConnect(" << peerAddress << ")");
  LOG_INFO("Got connection from device at " << peerAddress << ")");
  
  messageProcessor = new AndroidMessageProcessor(this);
  messageProcessor->activate();
}

void AndroidEventHandler::onDisconnect() {
  LOG_TRACE("onDisconnect()");
  
  LOG_TRACE((long) this << " Closing Message Processor");
  messageProcessor->close(0);
  LOG_TRACE((long) this << " Waiting for message processor thread to finish...");
  //FIXME:  ACE-specific hack in service handler
  this->serviceHandler->reactor()->lock().release(); //release the lock, or we'll hang if other threads try to do stuff with the reactor while we're waiting
  messageProcessor->wait();
  this->serviceHandler->reactor()->lock().acquire(); //and put it back like it was
  LOG_TRACE((long) this << " Message processor finished.");
}

int AndroidEventHandler::onMessageAvailable(ammo::protocol::MessageWrapper *msg) {
  LOG_TRACE("onMessageAvailable()");
  
  addReceivedMessage(msg, msg->message_priority());
  messageProcessor->signalNewMessageAvailable();
  
  return 0;
}

int AndroidEventHandler::onError(const char errorCode) {
  LOG_ERROR("onError(" << errorCode << ")");
  return 0;
}


AndroidEventHandler::~AndroidEventHandler() {
  LOG_DEBUG("AndroidEventHandler being destroyed!");
  
   delete messageProcessor;
}

ammo::protocol::MessageWrapper *AndroidEventHandler::getNextReceivedMessage() {
  ammo::protocol::MessageWrapper *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.top().message;
    receiveQueue.pop();
  }
  receiveQueueMutex.release();
  
  return msg;
}

void AndroidEventHandler::addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority) {
  QueuedMessage queuedMsg;
  queuedMsg.priority = priority;
  queuedMsg.message = msg;
  
  if(priority != msg->message_priority()) {
    LOG_WARN((long) this << " Priority mismatch on received message: Header = " << (int) priority << ", Message = " << msg->message_priority());
  }
  
  receiveQueueMutex.acquire();
  queuedMsg.messageCount = receivedMessageCount;
  receivedMessageCount++;
  receiveQueue.push(queuedMsg);
  receiveQueueMutex.release();
}
