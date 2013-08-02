#include "SerialWriterThread.h"

#include "SerialConnector.h"
#include "log.h"

SerialWriterThread::SerialWriterThread(SerialConnector *connector) : 
connector(connector),
closeMutex(),
closed(true),
sendQueueMutex(),
newMessageAvailable(sendQueueMutex) {

}

SerialWriterThread::~SerialWriterThread() {
  if(!isClosed()) {
    LOG_ERROR("SerialWriterThread: Entered destructor before stop()");
    stop();
    this->wait(); //don't finish destroying until our thread terminates
  }
}

int SerialWriterThread::svc() {
  closed = false;

  while(!isClosed()) {
    //do stuff
    QueuedMessagePtr messageToSend;
    while((messageToSend = getNextMessage())) {
      connector->writeMessageFragment(*messageToSend);
    }

    {
      ThreadMutexGuard g(sendQueueMutex);
      newMessageAvailable.wait();
    }
  }

  return 0;
}

void SerialWriterThread::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
  } else {
    LOG_ERROR("Error acquiring lock in SerialWriterThread::stop()")
  }
}

bool SerialWriterThread::isClosed() {
  volatile bool temp;
  {
    ThreadMutexGuard g(closeMutex);
    if(g.locked()) {
      temp = closed;
    } else {
      temp = false;
      LOG_ERROR("Error acquiring lock in SerialWriterThread::isClosed()")
    }
  }
  return temp;
}

void SerialWriterThread::queueMessage(QueuedMessagePtr message) {
  ThreadMutexGuard g(sendQueueMutex);
  sendQueue.push(message);
  LOG_TRACE("SENDER: queued message to send; " << sendQueue.size() << " messages in queue");
  newMessageAvailable.signal();
}

SerialWriterThread::QueuedMessagePtr SerialWriterThread::getNextMessage() {
  ThreadMutexGuard g(sendQueueMutex);
  if(!sendQueue.empty()) {
    QueuedMessagePtr nextMessage = sendQueue.front();
    sendQueue.pop();
    LOG_TRACE("SENDER: Got next message to send; " << sendQueue.size() << " messages remain in queue");
    return nextMessage;
  } else {
    LOG_TRACE("SENDER: Sender queue is empty.");
    return NULL;
  }
}
