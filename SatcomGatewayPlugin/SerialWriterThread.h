#ifndef SERIAL_WRITER_THREAD_H
#define SERIAL_WRITER_THREAD_H

#include <queue>
#include <string>

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>

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

  ACE_Thread_Mutex sendQueueMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  std::queue<QueuedMessagePtr> sendQueue;
  QueuedMessagePtr getNextMessage();
};

#endif //SERIAL_WRITER_THREAD_H
