#include <string>
#include <ctime>

#include "AndroidEventHandler.h"
#include "AndroidPluginConfigurationManager.h"
#include "NetworkServiceHandler.h"
#include "log.h"

#include "protocol/AmmoMessages.pb.h"
#include "AndroidMessageProcessor.h"

using namespace std;

AndroidEventHandler::AndroidEventHandler() :
messageProcessor(NULL),
receiveQueueMutex(),
receivedMessageCount(0) {
  LOG_TRACE((long) this << " ctor AndroidEventHandler()");
  latestMessageTime = time(NULL);
  heartbeatTimeoutTime = AndroidPluginConfigurationManager::getInstance()->getHeartbeatTimeout();
}

void AndroidEventHandler::onConnect(std::string &peerAddress) {
  LOG_TRACE((long) this << " AndroidEventHandler::onConnect(" << peerAddress << ")");
  LOG_INFO((long) this << " Got connection from device at " << peerAddress << ")");
  
  messageProcessor = new AndroidMessageProcessor(this);
  messageProcessor->activate();
  latestMessageTime = time(NULL);
}

void AndroidEventHandler::onDisconnect() {
  LOG_TRACE((long) this << " AndroidEventHandler::onDisconnect()");
  
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
  LOG_TRACE((long) this << " AndroidEventHandler::onMessageAvailable()");
  
  addReceivedMessage(msg, msg->message_priority());
  messageProcessor->signalNewMessageAvailable();
  
  latestMessageTime = time(NULL);
  
  return 0;
}

int AndroidEventHandler::onError(const char errorCode) {
  LOG_ERROR((long) this << " AndroidEventHandler::onError(" << errorCode << ")");
  return 0;
}


AndroidEventHandler::~AndroidEventHandler() {
  LOG_DEBUG((long) this << " AndroidEventHandler being destroyed!");
  
  //Flush the receive queue
  int count = 0;
  while(!receiveQueue.empty()) {
    ProtobufMessageWrapper *msg = sendQueue.top().message;
    delete msg;
    count++;
  }
  LOG_TRACE((long) this << " " << count << "messages flushed from receive queue");
  
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

/**
* Wraps NetworkEventHandler::sendMessage, extending it to check for queue length
* and time since last heartbeat (so we can expire stale connections).  We wrap
* sendMessage rather than making it virtual for performance reasons; since
* NetworkEventHandler is used in other places in the gateway codebase, we don't
* want to cause that kind of performance impact everywhere.
*/
void AndroidEventHandler::send(ammo::protocol::MessageWrapper *msg) {
  //Check heartbeat time
  time_t heartbeatDelta = time(NULL) - latestMessageTime;
  if(heartbeatDelta > heartbeatTimeoutTime) {
    LOG_WARN((long) this << " Haven't received a message from device since " << latestMessageTime << " (" << heartbeatDelta << " seconds ago");
    LOG_WARN((long) this << "   Dropping connection to device.");
    this->close();
    delete msg; //we have ownership of msg, so we need to delete it (it isn't
                //going into the send message queue if we drop the connection)
    return;
  }
  
  //TODO: Check message queue length
  
  this->sendMessage(msg);
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
