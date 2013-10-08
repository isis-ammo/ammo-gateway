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
    if(messageToSend) {
      connector->writeMessageFragment(*messageToSend);
    } else {
      if(!isClosed()) { //this is the normal path if we're in the process of closing, so don't print the warning
        LOG_WARN("Tried to send empty fragment");
      }
    }
  }

  return 0;
}

void SerialWriterThread::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
    {
      //unblocks the svc() loop so we can terminate
      newMessagesAvailableCount.release();
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
