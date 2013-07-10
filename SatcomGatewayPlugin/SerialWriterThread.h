#ifndef SERIAL_READER_THREAD_H
#define SERIAL_READER_THREAD_H

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>

class SerialConnector;

class SerialWriterThread : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
public:
  SerialWriterThread(SerialConnector *connector);
  virtual ~SerialWriterThread();

  virtual int svc();

  void stop();

private:
  SerialConnector *connector;

  typedef ACE_Guard<ACE_Thread_Mutex> ThreadMutexGuard;
  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();
};

#endif //SERIAL_READER_THREAD_H
