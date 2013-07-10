#include "SerialWriterThread.h"

#include "SerialConnector.h"
#include "log.h"

SerialWriterThread::SerialWriterThread(SerialConnector *connector) : 
connector(connector),
closeMutex(),
closed(true) {

}

SerialWriterThread::~SerialWriterThread() {
  if(!isClosed()) {
    LOG_ERROR("SerialWriterThread: Entered destructor before stop()");
    stop();
    this->wait(); //don't finish destroying until we're not running anything in background
  }
}

int SerialWriterThread::svc() {
  while(!isClosed()) {
    //do stuff
  }

  return 0;
}

void SerialWriterThread::stop() {
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
}

bool SerialWriterThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}