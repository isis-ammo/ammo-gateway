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
    this->wait(); //don't finish destroying until we're not running anything in background
  }
}

int SerialReaderThread::svc() {
  while(!isClosed()) {
    //do stuff
  }

  return 0;
}

void SerialReaderThread::stop() {
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
}

bool SerialReaderThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}