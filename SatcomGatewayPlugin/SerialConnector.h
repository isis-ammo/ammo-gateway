/* Copyright (C) 2010-2013 Vanderbilt University
 *  
 * This software was developed by the Institute for Software Integrated
 * Systems (ISIS) at Vanderbilt University, Tennessee, USA for the
 * Transformative Apps program under DARPA, Contract # HR011-10-C-0175 and
 * Contract # HR0011-12-C-0109.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this data, including any software or models in source or binary
 * form, as well as any drawings, specifications, and documentation
 * (collectively "the Data"), to deal in the Data without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Data, and to
 * permit persons to whom the Data is furnished to do so, subject to the
 * following conditions:
 *  
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Data.
 *  
 * THE DATA IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS, SPONSORS, DEVELOPERS, CONTRIBUTORS, OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE DATA OR THE USE OR OTHER DEALINGS IN THE DATA. 
 */

#ifndef SERIAL_CONNECTOR_H
#define SERIAL_CONNECTOR_H

#include <stdint.h>
#include <vector>
#include <queue>
#include <iostream>

#include <ace/DEV_Connector.h>
#include <ace/TTY_IO.h>
#include <ace/Copy_Disabled.h>
#include <ace/Task.h>

#include <GatewayConnector.h>

#include "Typedefs.h"
#include "SerialReaderThread.h"
#include "SerialWriterThread.h"
#include "TerseDecoder.h"


const uint32_t MAGIC_NUMBER = 0xabad1dea;

const uint8_t * const MAGIC_NUMBER_BYTES = reinterpret_cast<const uint8_t *>(&MAGIC_NUMBER);

const uint8_t GATEWAY_SENDER_ID = 0;

//binary serialization helper methods; used in a couple classes here
void appendUInt8(std::ostream &stream, const uint8_t val);
void appendUInt16(std::ostream &stream, const uint16_t val);

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

struct DataMessageFragment {
public:
  typedef std::tr1::shared_ptr<const std::string> MessageDataPtr;

  DataMessageFragment();

  MessageDataPtr serializeFragment();

  uint16_t sequenceNumber;
  uint16_t index;
  uint16_t count;

  bool shouldAck;

  MessageDataPtr messageData;
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

  uint16_t getStartingSequenceNumber() { return startingSequenceNumber; }
  uint16_t getFragmentsCount() { return fragmentsCount; }
  uint16_t getReceivedFragmentsCount() { return receivedFragmentsCount; }

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



class SerialConnector : public ACE_Task<ACE_MT_SYNCH>, public ACE_Copy_Disabled, public ammo::gateway::GatewayConnectorDelegate, public ammo::gateway::DataPushReceiverListener {
private:
  typedef std::queue<uint16_t> SequenceNumberQueue;

public:
  SerialConnector();
  virtual ~SerialConnector();
  
  virtual int svc();
  void stop();

  ssize_t receiveN(void *buf, size_t n, ACE_Time_Value *timeout);

  bool writeMessageFragment(const std::string &message);

  void receivedMessageFragment(const DataMessage dataHeader, const uint8_t shouldAck, const uint8_t dataType, const std::string &data);
  void receivedAckPacket(const bool isToken, const uint8_t ackCount, const std::vector<uint16_t> &acks);
  void receivedReset();

  SequenceNumberQueue getSequenceNumbersToAck();

  //GatewayConnectorDelegate methods
  virtual void onConnect(ammo::gateway::GatewayConnector *sender);
  virtual void onDisconnect(ammo::gateway::GatewayConnector *sender);

  //DataPushReceiverListener methods
  virtual void onPushDataReceived(ammo::gateway::GatewayConnector *sender, ammo::gateway::PushData &pushData);
  
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

  void processMessage(const uint8_t dataType, const std::string &message);

  ammo::gateway::GatewayConnector gatewayConnector;

  SerialReaderThread reader;
  SerialWriterThread writer;

  TerseDecoder terseDecoder;

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

  ACE_Thread_Mutex sendDataMapMutex;
  typedef std::map<uint16_t, DataMessageFragment> SendDataMap;
  SendDataMap sendDataMap;
  uint16_t nextSequenceNumber;
  void enqueueDataMessage(const std::string &message, const bool shouldAck);

  int tokenTimeout;
  std::string initialState;
  uint16_t fragmentSize;
  int fragmentsToSendPerCycle;

  uint8_t myTokenSequenceNumber;
  uint8_t lastReceivedTokenSequenceNumber;

  void reset();

  void sendResetAck();
  void sendAckPacket();
  void sendTokenPacket();
  void sendDataFragments();

  typedef std::tr1::unordered_map<uint16_t, FragmentedMessage> IncompleteMessageMap;
  IncompleteMessageMap incompleteMessages; //indexed by first sequence number in message

  

  friend std::ostream& operator<<( std::ostream& stream, const SerialConnector::SerialConnectorEvent &ev );
  friend std::ostream& operator<<( std::ostream& stream, const SerialConnector::SerialConnectorState &ev );
};

inline ssize_t SerialConnector::receiveN(void *buf, size_t n, ACE_Time_Value *timeout) {
  return serialDevice.recv_n(buf, n, timeout);
}

inline std::ostream& operator<<( std::ostream& stream, const SerialConnector::SerialConnectorEvent &ev )
{
    switch(ev) {
    case SerialConnector::EVENT_NONE:
      stream << "EVENT_NONE";
      break;
    case SerialConnector::EVENT_MESSAGE_RECEIVED:
      stream << "EVENT_MESSAGE_RECEIVED";
      break;
    case SerialConnector::EVENT_TOKEN_RECEIVED:
      stream << "EVENT_TOKEN_RECEIVED";
      break;
    case SerialConnector::EVENT_RESET_RECEIVED:
      stream << "EVENT_RESET_RECEIVED";
      break;
    case SerialConnector::EVENT_TIMEOUT:
      stream << "EVENT_TIMEOUT";
      break;
    case SerialConnector::EVENT_CLOSE:
      stream << "EVENT_CLOSE";
      break;
    default:
      stream << "Unknown Event (" << ev << ")";
      break;
    }
    return stream;
}

inline std::ostream& operator<<( std::ostream& stream, const SerialConnector::SerialConnectorState &ev )
{
    switch(ev) {
    case SerialConnector::STATE_RECEIVING:
      stream << "Receiving";
      break;
    case SerialConnector::STATE_SENDING:
      stream << "Sending";
      break;
    case SerialConnector::STATE_WAITING_FOR_ACK:
      stream << "Waiting for ack";
      break;
    default:
      stream << "Unknown state (" << ev << ")";
      break;
    }
    return stream;
}

#endif //SERIAL_CONNECTOR_H
