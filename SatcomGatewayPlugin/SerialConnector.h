#ifndef SERIAL_CONNECTOR_H
#define SERIAL_CONNECTOR_H

#include <stdint.h>

#include "ace/DEV_Connector.h"
#include "ace/TTY_IO.h"
#include "ace/Copy_Disabled.h"
#include <ace/Task.h>

const uint32_t MAGIC_NUMBER = 0xabad1dea;

const uint8_t * const MAGIC_NUMBER_BYTES = reinterpret_cast<const uint8_t *>(&MAGIC_NUMBER);

struct SatcomHeader {
  uint32_t magicNumber;
  uint16_t size;
  uint16_t reserved;
  uint16_t payloadChecksum;
  uint16_t headerChecksum;
};

const uint8_t MESSAGE_TYPE_DATA = 0;
const uint8_t MESSAGE_TYPE_ACK_TOKEN = 1;

struct DataMessage {
  uint16_t sequenceNumber;
  uint16_t index;
  uint16_t count;
};



class SerialConnector : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
public:
  SerialConnector();
  virtual ~SerialConnector();
  
  virtual int svc();
  void stop();

  char readChar();
  
private:
  bool connect();
  
  typedef ACE_Guard<ACE_Thread_Mutex> ThreadMutexGuard;
  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();
  
  ACE_TTY_IO serialDevice;
  ACE_DEV_Connector serialConnector;
};

#endif //SERIAL_CONNECTOR_H
