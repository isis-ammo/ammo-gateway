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

#ifndef SERIAL_WRITER_THREAD_H
#define SERIAL_WRITER_THREAD_H

#include <queue>
#include <string>

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>
#include <ace/Thread_Semaphore.h>

#include "Typedefs.h"

class SerialConnector;

class SerialWriterThread : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
public:
  typedef std::tr1::shared_ptr<const std::string> QueuedMessagePtr;
  typedef std::tr1::shared_ptr<std::string> MutableQueuedMessagePtr; //convenience for consumers of this class

  SerialWriterThread(SerialConnector *connector);
  virtual ~SerialWriterThread();

  virtual int svc();

  void stop();

  void queueMessage(QueuedMessagePtr message);

private:
  SerialConnector *connector;

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();

  ACE_Thread_Semaphore newMessagesAvailableCount;

  ACE_Thread_Mutex sendQueueMutex;
  std::queue<QueuedMessagePtr> sendQueue;
  QueuedMessagePtr getNextMessage();

  
};

#endif //SERIAL_WRITER_THREAD_H
