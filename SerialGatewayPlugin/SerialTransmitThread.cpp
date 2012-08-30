/*
 * SerialTransmitThread.cpp
 *
 *  Created on: Aug 28, 2012
 *      Author: jwilliams
 */

#include "SerialTransmitThread.h"
#include "SerialConfigurationManager.h"
#include "GpsThread.h"
#include "GatewayReceiver.h"
#include "SerialServiceHandler.h"

#include "ace/Time_Value.h"
#include <cmath>

const int BAUD_RATE = 9600;

const uint8_t MAGIC[] = { 0xef, 0xbe, 0xed };

const uint8_t VERSION = 0x40; //gets OR'd with the slot number to produce TerseMessageHeader::versionAndSlot

SerialTransmitThread::SerialTransmitThread(SerialServiceHandler *parent, GatewayReceiver *receiver, GpsThread *gpsThread) : parent(parent), receiver(receiver), gpsThread(gpsThread), closed(false), newMessageAvailable(newMessageMutex) {
  // TODO Auto-generated constructor stub
  SerialConfigurationManager *config = SerialConfigurationManager::getInstance();
  slotDuration = config->getSlotDuration();
  slotNumber = config->getSlotNumber();
  numberOfSlots = config->getNumberOfSlots();
  transmitDuration = config->getTransmitDuration();
  gpsTimeOffset = config->getGpsTimeOffset();
}

SerialTransmitThread::~SerialTransmitThread() {
  // TODO Auto-generated destructor stub
}

void SerialTransmitThread::stop() {
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
}

int SerialTransmitThread::svc() {
  int offset = ((slotNumber - 1) % numberOfSlots) * slotDuration;
  int cycleDuration = slotDuration * numberOfSlots;
  double bytesPerMs = BAUD_RATE / (10*1000.0);

  long tweakedTransmitDuration = transmitDuration + (long) std::min(50, transmitDuration / 10);
  long maxPayloadSize = (long) (transmitDuration * bytesPerMs);

  while(!isClosed()) {
    long systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
    long gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;

    long cycleStartTime = (long) (gpsTime / cycleDuration) * cycleDuration;
    long thisSlotBegin = cycleStartTime + offset;
    long thisSlotEnd = thisSlotBegin + tweakedTransmitDuration;

    long thisSlotConsumed = 0;

    if(gpsTime < thisSlotBegin) {
      //sleep until slot begins
      usleep((thisSlotBegin - gpsTime) * 1000);
    } else if(gpsTime > thisSlotEnd) {
      //missed our slot...  sleep until next slot begins
      usleep((thisSlotBegin + cycleDuration - gpsTime) * 1000);
    } else {
      //we're in our slot...  send as much data as we can
      //LOG_DEBUG("In slot... " << thisSlotEnd - gpsTime << " remaining in slot");
      bool slotTimeAvailable = true;

      while(slotTimeAvailable && receiver->isMessageAvailable()) {
        int nextMessageLength = receiver->getNextMessageSize() + sizeof(TerseMessageHeader);
        if(nextMessageLength > maxPayloadSize) {
          //throw too-long messages out without processing
          LOG_WARN("Message of size " << nextMessageLength << " is greater than max payload size... dropping.");
          std::string *msg = receiver->getNextReceivedMessage();
          delete msg;
        } else {
          //update GPS time; previous operation might have taken a while
          systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
          gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;

          long timeLeft = thisSlotEnd - gpsTime;
          long bytesThatWillFit = timeLeft * bytesPerMs;

          LOG_TRACE("Time: " << gpsTime << ", left in slot " << timeLeft << "ms, bytes sent " << thisSlotConsumed << "/" << maxPayloadSize);
          if(nextMessageLength <= (maxPayloadSize - thisSlotConsumed) && nextMessageLength <= bytesThatWillFit) {
            //send the message
            std::string *msg = receiver->getNextReceivedMessage();
            LOG_TRACE("Sending message, length " << msg->length() << " + header");
            sendMessage(msg);
            thisSlotConsumed += nextMessageLength;
            delete msg;
          } else {
            //hold the message until the next slot (we process messages in order, so we can't skip ahead)
            LOG_TRACE("Out of slot space!");
            slotTimeAvailable = false;
          }
        }
      }
      //finished sending data in this slot...  sleep until the next one
      systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
      gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;

      usleep((thisSlotBegin + cycleDuration - gpsTime) * 1000);
    }
  }
  return 0;
}

void SerialTransmitThread::sendMessage(std::string *msg) {
  //build the message header
  TerseMessageHeader header;
  header.magic[0] = MAGIC[0];
  header.magic[1] = MAGIC[1];
  header.magic[2] = MAGIC[2];

  header.versionAndSlot = VERSION | slotNumber;
  header.payloadSize = msg->length();

  uint32_t fullChecksum = ACE::crc32(msg->data(), msg->length());
  header.payloadChecksum = static_cast<uint16_t>(fullChecksum);

  long systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
  long gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;
  int gpsTimeInt = static_cast<int>(gpsTime % 100000000);
  header.timestamp = gpsTimeInt;
  header.reserved = 0;

  uint32_t headerChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));
  header.headerChecksum = (uint16_t) headerChecksum;

  //send the header
  for(int i = 0; i < sizeof(header); i++) {
    parent->write_a_char(reinterpret_cast<uint8_t *>(&header)[i]);
  }
  //send the payload
  for(int i = 0; i < msg->length(); i++) {
    parent->write_a_char((*msg)[i]);
  }
}

bool SerialTransmitThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}
