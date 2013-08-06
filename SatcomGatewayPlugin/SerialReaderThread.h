#ifndef SERIAL_READER_THREAD_H
#define SERIAL_READER_THREAD_H

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>
#include <vector>
#include <iostream>

#include "Typedefs.h"

class SerialConnector;
struct SatcomHeader;

class SerialReaderThread : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
public:
  SerialReaderThread(SerialConnector *connector);
  virtual ~SerialReaderThread();

  virtual int svc();

  void stop();

private:
  enum ReaderThreadState {
    STATE_READING_MAGIC,
    STATE_READING_HEADER,
    STATE_READING_DATA
  };

  bool validateHeaderChecksum(const SatcomHeader &header);

  bool readAChar(uint8_t &result);

  bool processData(const SatcomHeader &header, const std::string &payload);

  SerialConnector *connector;

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();

  friend std::ostream& operator<<( std::ostream& stream, const SerialReaderThread::ReaderThreadState &state );
};

inline std::ostream& operator<<( std::ostream& stream, const SerialReaderThread::ReaderThreadState &state )
{
    switch(state) {
    case SerialReaderThread::STATE_READING_MAGIC:
      stream << "Reading magic";
      break;
    case SerialReaderThread::STATE_READING_HEADER:
      stream << "Reading header";
      break;
    case SerialReaderThread::STATE_READING_DATA:
      stream << "Reading data";
      break;
    default:
      stream << "Unknown state (" << state << ")";
      break;
    }
    return stream;
}

#endif //SERIAL_READER_THREAD_H
