#include "SerialReaderThread.h"

#include "SerialConnector.h"
#include "log.h"

SerialReaderThread::SerialReaderThread(SerialConnector *connector) : 
connector(connector),
closeMutex(),
closed(true) {

}

SerialReaderThread::~SerialReaderThread() {
  if(!isClosed()) {
    LOG_ERROR("SerialReaderThread: Entered destructor before stop()");
    stop();
    this->wait(); //don't finish destroying until our thread terminates
  }
}

int SerialReaderThread::svc() {
  while(!isClosed()) {
    //do stuff
  }

  return 0;
}

void SerialReaderThread::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
  } else {
    LOG_ERROR("Error acquiring lock in SerialReaderThread::stop()")
  }
}

bool SerialReaderThread::isClosed() {
  volatile bool temp;
  {
    ThreadMutexGuard g(closeMutex);
    if(g.locked()) {
      temp = closed;
    } else {
      temp = false;
      LOG_ERROR("Error acquiring lock in SerialReaderThread::isClosed()")
    }
  }
  return temp;
}