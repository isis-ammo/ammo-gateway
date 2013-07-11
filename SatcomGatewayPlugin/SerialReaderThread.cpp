#include "SerialReaderThread.h"

#include "SerialConnector.h"
#include "log.h"

#include <tr1/memory>

using namespace std::tr1;
using namespace std;

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
  ReaderThreadState state = STATE_READING_MAGIC;
  size_t bytesRead = 0;

  SatcomHeader header;

  std::vector<uint8_t> payload;


  while(!isClosed()) {
    uint8_t readChar = connector->readChar();

    if(state == STATE_READING_MAGIC) {
      if(readChar == MAGIC_NUMBER_BYTES[bytesRead]) {
        reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
        bytesRead++;
        if(bytesRead >= sizeof(MAGIC_NUMBER)) {
          state = STATE_READING_HEADER;
        }
      } else {
        //got an invalid byte; reset and start over
        LOG_WARN("Received bad magic number");
        state = STATE_READING_MAGIC;
        bytesRead = 0;
      }
    } else if(state == STATE_READING_HEADER) {
      reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
      bytesRead++;
      if(bytesRead >= sizeof(header)) {
        bool headerChecksumValid = validateHeaderChecksum(header);
        if(headerChecksumValid) {
          state = STATE_READING_DATA;
          payload.clear();
          payload.resize(header.size);
          bytesRead = 0;
        } else {
          //received header was corrupt; start over
          LOG_WARN("Received corrupt header");
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }
      } else if(state == STATE_READING_DATA) {
        payload[bytesRead] = readChar;
        bytesRead++;

        if(bytesRead >= header.size) {
          //got the whole message; pass it on to the processor
          processData(header, payload);
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }

      }
    }
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

bool SerialReaderThread::validateHeaderChecksum(const SatcomHeader &header) {
  uint32_t expectedChecksum = header.headerChecksum;
  uint32_t calculatedChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  bool result = (calculatedChecksum & 0xffff) == (expectedChecksum & 0xffff);
  return result;
}

void SerialReaderThread::processData(const SatcomHeader &header, const vector<uint8_t> &payload) {
  //to be implemented
}
