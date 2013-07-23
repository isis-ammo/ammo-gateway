#ifndef SERIAL_CONNECTOR_H
#define SERIAL_CONNECTOR_H

#include <stdint.h>
#include <vector>
#include <queue>

#include <ace/DEV_Connector.h>
#include <ace/TTY_IO.h>
#include <ace/Copy_Disabled.h>
#include <ace/Task.h>

#include "Typedefs.h"


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

class FragmentedMessage {
public:
  typedef std::tr1::shared_ptr<const std::string> FragmentedMessageDataPtr;

  FragmentedMessage(uint16_t startingSequenceNumber, uint16_t fragmentsCount, uint8_t dataType) : startingSequenceNumber(startingSequenceNumber), fragmentsCount(fragmentsCount), receivedFragmentsCount(0), dataType(dataType), fragments(fragmentsCount) {};

  void gotMessageFragment(const DataMessage dataHeader, const std::string &data);

  bool isMessageComplete() {
    return receivedFragmentsCount == fragmentsCount;
  }

  uint8_t getDataType() {
    return dataType;
  }

  /**
  * Precondition: complete message has been received (isMessageComplete() is true).
  * Behavior is undefined if it hasn't.
  */
  std::string reconstructCompleteMessage();
private:

  uint16_t startingSequenceNumber;
  uint16_t fragmentsCount;
  uint16_t receivedFragmentsCount;
  uint8_t dataType;

  typedef std::vector<FragmentedMessageDataPtr> FragmentVector;
  FragmentVector fragments;
};



class SerialConnector : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled {
private:
  typedef std::queue<uint16_t> SequenceNumberQueue;

public:
  SerialConnector();
  virtual ~SerialConnector();
  
  virtual int svc();
  void stop();

  char readChar();
  bool writeMessageFragment(const std::string &message);

  void receivedMessageFragment(const DataMessage dataHeader, const uint8_t shouldAck, const uint8_t dataType, const std::string &data);
  void receivedAckPacket(const bool isToken, const std::vector<uint16_t> &acks);

  SequenceNumberQueue getSequenceNumbersToAck();
  
private:
  enum SerialConnectorState {
    STATE_RECEIVING,
    STATE_SENDING,
    STATE_WAITING_FOR_ACK
  };

  enum SerialConnectorEvent {
    EVENT_NONE,
    EVENT_MESSAGE_RECEIVED,
    EVENT_TOKEN_RECEIVED,
    EVENT_RESET_RECEIVED,
    EVENT_TIMEOUT,
    EVENT_CLOSE
  };

  bool connect();

  void processMessage(std::string &message);

  ACE_Thread_Mutex closeMutex;
  bool closed;
  bool isClosed();

  ACE_Thread_Mutex eventMutex;
  ACE_Condition_Thread_Mutex eventCondition;
  volatile SerialConnectorEvent lastSignaledEvent;
  void signalEvent(SerialConnectorEvent ev);
  SerialConnectorEvent waitForEventSignal(int timeoutMilliseconds = 0); //0 timeout will wait forever
  
  ACE_TTY_IO serialDevice;
  ACE_DEV_Connector serialConnector;

  SequenceNumberQueue sequenceNumbersToAck;
  ACE_Thread_Mutex sequenceNumbersToAckMutex;

  typedef std::tr1::unordered_map<uint16_t, FragmentedMessage> IncompleteMessageMap;
  IncompleteMessageMap incompleteMessages; //indexed by first sequence number in message
};

#endif //SERIAL_CONNECTOR_H
