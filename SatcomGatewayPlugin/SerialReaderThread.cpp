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

#include "SerialReaderThread.h"

#include <sstream>

#include "SerialConnector.h"
#include "log.h"

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
  closed = false;
  ReaderThreadState state = STATE_READING_MAGIC;
  size_t bytesRead = 0;

  SatcomHeader header;

  std::string payload;


  while(!isClosed()) {
    //LOG_TRACE("Receiver looping in svc(), state " << state);
    uint8_t readChar = 0;
    bool result = readAChar(readChar);

    if(result == true) {
      if(state == STATE_READING_MAGIC) {
        if(readChar == MAGIC_NUMBER_BYTES[bytesRead]) {
          reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
          bytesRead++;
          if(bytesRead >= sizeof(MAGIC_NUMBER)) {
            state = STATE_READING_HEADER;
          }
        } else {
          //got an invalid byte; reset and start over
          LOG_WARN("Received bad magic number (" << ((unsigned int) readChar & 0xff) << ")");
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }
      } else if(state == STATE_READING_HEADER) {
        reinterpret_cast<uint8_t *>(&header)[bytesRead] = readChar;
        bytesRead++;
        if(bytesRead >= sizeof(header)) {
          LOG_TRACE("Validating header checksum");
          bool headerChecksumValid = validateHeaderChecksum(header);
          if(headerChecksumValid) {
            state = STATE_READING_DATA;
            payload.clear();
            LOG_TRACE("Header checksum valid; allocating buffer of size " << header.size);
            payload.resize(header.size);
            bytesRead = 0;
          } else {
            //received header was corrupt; start over
            LOG_WARN("Received corrupt header");
            state = STATE_READING_MAGIC;
            bytesRead = 0;
          }
        }
      } else if(state == STATE_READING_DATA) {
        payload[bytesRead] = readChar;
        bytesRead++;
        //LOG_TRACE("Read " << bytesRead << " bytes");

        if(bytesRead >= header.size) {
          //got the whole message; pass it on to the processor
          processData(header, payload);
          state = STATE_READING_MAGIC;
          bytesRead = 0;
        }

      } else {
        LOG_ERROR("*** Unknown state " << state << " ***");
      }
    } else {
      LOG_DEBUG("readChar returned false; didn't receive any data");
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

bool SerialReaderThread::readAChar(uint8_t &result) {
  //LOG_TRACE("In SerialReaderThread::readChar()");
  uint8_t temp;
  
  ssize_t count = 0;
  while(!isClosed() && (count == 0 || (count == -1 && errno == ETIME))) {
    ACE_Time_Value timeout(0, 10000);
    //LOG_TRACE("Calling recv_n");
    count = connector->receiveN((void *) &temp, 1, &timeout);
  }

  if(isClosed()) {
    return false;
  }

  if ( count == -1 )
  {
    return false;
  }
  else if ( count >= 1 )
  {

  }
  else if ( count == 0 )
  {
    LOG_ERROR("Read returned 0" );
    return false;
  }

  //LOG_TRACE("Serial Received " << std::hex << (int) temp);
  result = temp;
  return true;
}

bool SerialReaderThread::validateHeaderChecksum(const SatcomHeader &header) {
  uint32_t expectedChecksum = header.headerChecksum;
  uint32_t calculatedChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));

  bool result = (calculatedChecksum & 0xffff) == (expectedChecksum & 0xffff);
  return result;
}

/*
* Message Type bitfield structure:
* abcd eeee
* 
* a: Packet type (0 = data, 1 = ack or token)
* b: Reset (0 = normal, 1 = reset)
* c: should ack (0 = don't ack; 1 = do ack)
* d: sender ID (0 = gateway; 1 = device); messages from gateway should be ignored (over these radios, everything we send is echoed back)
* eeee: Unused (set to 0)
*/
bool SerialReaderThread::processData(const SatcomHeader &header, const std::string &payload) {
  LOG_TRACE("in processData()");

  if(payload.size() > 0) {
    uint32_t expectedChecksum = header.payloadChecksum;
    uint32_t calculatedChecksum = ACE::crc32(payload.data(), payload.length());

    bool payloadChecksumValid = (calculatedChecksum & 0xffff) == (expectedChecksum & 0xffff);

    if(payloadChecksumValid) {
      //get packet type (in the high bit of the first byte of the payload)
      uint8_t payloadInfoByte = payload[0];
      uint8_t payloadType = payloadInfoByte >> 7;
      uint8_t isReset = (payloadInfoByte >> 6) & 0x01;
      uint8_t senderId = (payloadInfoByte >> 4) & 0x01;

      if(isReset) {
        LOG_TRACE("Received reset packet");
        connector->receivedReset();
        return true;
      }

      switch(payloadType) {
        case MESSAGE_TYPE_DATA: {
          LOG_TRACE("Received data packet");
          if((payload.size() - sizeof(payloadInfoByte)) >= sizeof(DataMessage)) {
            uint8_t shouldAck = (payloadInfoByte >> 5) & 0x01;
            uint8_t dataType = payloadInfoByte & 0x1f;

            DataMessage header = * reinterpret_cast<const DataMessage *>((&payload[sizeof(payloadInfoByte)]));
            const string data(payload.begin() + sizeof(payloadInfoByte) + sizeof(DataMessage), payload.end());

            connector->receivedMessageFragment(header, shouldAck, dataType, data);

            return true;
          } else {
            LOG_ERROR("Payload doesn't contain enough data to extract DataHeader");
            return false;
          }
          break;
        }
        case MESSAGE_TYPE_ACK_TOKEN: {
          LOG_TRACE("Received ack/token packet");
          if((payload.size() - sizeof(payloadInfoByte)) >= sizeof(uint16_t)) {
            
            uint16_t ackCountShort = * reinterpret_cast<const uint16_t *>(&payload[sizeof(payloadInfoByte)]);
            LOG_TRACE("Ack count short: " << std::hex << ackCountShort);
            bool isToken = (ackCountShort >> 15) == 1;
            uint16_t ackCount = ackCountShort & 0x7f;
            vector<uint16_t> ackSequenceNumbers;
            if(!isToken) {
              size_t expectedPayloadSize = ackCount * sizeof(uint16_t) + sizeof(ackCountShort) + sizeof(payloadInfoByte);

              if(expectedPayloadSize == payload.size()) {
                ackSequenceNumbers.reserve(ackCount);
                for(int i = 0; i < ackCount; i++) {
                  uint16_t newAckSequenceNumber = payload[i*sizeof(uint16_t) + sizeof(ackCountShort) + sizeof(payloadInfoByte)];
                  ackSequenceNumbers.push_back(newAckSequenceNumber);
                }
              }

              connector->receivedAckPacket(isToken, ackSequenceNumbers);
              return true;
            } else {
              LOG_ERROR("Ack packet is not the correct size...  (expected = " << expectedPayloadSize << " actual = " << payload.size() << ")");
              return false;
            }
          } else {
            LOG_ERROR("Not enough data to read ack byte");
            return false;
          }
          break;
        }
        default: {
          //shouldn't be possible; we're only working with one bit here...
          LOG_ERROR("Invalid message type: " << payloadType);
          return false;
        }
      }
    } else {
      LOG_ERROR("Incorrect payload checksum (received a corrupt packet) (Expected: " << std::hex << expectedChecksum << ", Calculated: " << calculatedChecksum << ")");
      return false;
    }
  } else {
    LOG_ERROR("Message with empty payload...");
    return false;
  }
}
