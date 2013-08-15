#include "SerialWriterThread.h"

#include "SerialConnector.h"
#include "log.h"

SerialWriterThread::SerialWriterThread(SerialConnector *connector) : 
connector(connector),
closeMutex(),
closed(true),
newMessagesAvailableCount(0),
sendQueueMutex() {

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
    //decrement the semaphore containing the count of new messages; if this is zero, we should block
    newMessagesAvailableCount.acquire();
    QueuedMessagePtr messageToSend;
    messageToSend = getNextMessage();
    connector->writeMessageFragment(*messageToSend);
  }

  return 0;
}

void SerialWriterThread::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
    {
      //unblocks the svc() loop so we can terminate
      //ThreadMutexGuard g(newMessageAvailableMutex);
      //newMessageAvailable.signal();
    }
  } else {
    LOG_ERROR("Error acquiring lock in SerialWriterThread::stop()");
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
      LOG_ERROR("Error acquiring lock in SerialWriterThread::isClosed()");
    }
  }
  return temp;
}

void SerialWriterThread::queueMessage(QueuedMessagePtr message) {
  {
    ThreadMutexGuard g(sendQueueMutex);

    if(g.locked()) {
      sendQueue.push(message);
      LOG_TRACE("SENDER: queued message to send; " << sendQueue.size() << " messages in queue");
    } else {
      LOG_ERROR("Error acquiring lock in SerialWriterThread::queueMessage()");
    }
  }

  //using this semaphore as a count of messages we have to send; release() increments it
  newMessagesAvailableCount.release();
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
    return QueuedMessagePtr();
  }
}
