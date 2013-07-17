#ifndef SERIAL_READER_THREAD_H
#define SERIAL_READER_THREAD_H

#include <ace/Task.h>
#include <ace/Copy_Disabled.h>
#include <vector>

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

  bool processData(const SatcomHeader &header, const std::vector<uint8_t> &payload);

  SerialConnector *connector;

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();
};

#endif //SERIAL_READER_THREAD_H
