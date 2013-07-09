#include "SerialConnector.h"

SerialConnector::SerialConnector() :
running(false),
serialDevice(),
serialConnector()
{
  
}

SerialConnector::~SerialConnector() {
  if(running) {
    stop();
  }
}

void SerialConnector::run() {
  running = true;
}

void SerialConnector::stop() {
  
  running = false;
}
