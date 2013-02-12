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
#include <stdint.h>

const uint8_t MAGIC[] = { 0xef, 0xbe, 0xed };

const uint8_t VERSION = 0x40; //gets OR'd with the slot number to produce SerialMessageHeader::versionAndSlot

//Windows doesn't have usleep, so we'll define it locally here
#ifdef WIN32
void usleep(int64_t useconds) {
  Sleep(useconds / 1000);
}
#endif

SerialTransmitThread::SerialTransmitThread(SerialServiceHandler *parent, GatewayReceiver *receiver, GpsThread *gpsThread) : parent(parent), receiver(receiver), gpsThread(gpsThread), closed(false), newMessageAvailable(newMessageMutex) {
  // TODO Auto-generated constructor stub
  SerialConfigurationManager *config = SerialConfigurationManager::getInstance();
  baudRate = config->getBaudRate();
  slotDuration = config->getSlotDuration();
  slotNumber = config->getSlotNumber();
  numberOfSlots = config->getNumberOfSlots();
  transmitDuration = config->getTransmitDuration();
  gpsTimeOffset = config->getGpsTimeOffset();
  pliSendFrequency = config->getPliSendFrequency();
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
  LOG_DEBUG("Serial transmit thread running");

  int offset = ((slotNumber - 1) % numberOfSlots) * slotDuration;
  int cycleDuration = slotDuration * numberOfSlots;
  double bytesPerMs = baudRate / (10*1000.0);

  int64_t tweakedTransmitDuration = transmitDuration + (int64_t) std::min(50, transmitDuration / 10);
  int64_t maxPayloadSize = (int64_t) (transmitDuration * bytesPerMs);

  bool sentPliRelayThisCycle = false;
  int pliCycleCount = 0;

  while(!isClosed()) {
    int64_t systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
    int64_t gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;
    LOG_TRACE("S:" << systemTime << " G:" << gpsTime);

    int64_t cycleStartTime = (int64_t) (gpsTime / cycleDuration) * cycleDuration;
    int64_t thisSlotBegin = cycleStartTime + offset;
    int64_t thisSlotEnd = thisSlotBegin + tweakedTransmitDuration;

    int64_t thisSlotConsumed = 0;

    if(gpsTime < thisSlotBegin) {
      //sleep until slot begins
      LOG_TRACE("Waiting");
      usleep((thisSlotBegin - gpsTime) * 1000);
    } else if(gpsTime > thisSlotEnd) {
      //missed our slot...  sleep until next slot begins
      LOG_TRACE("Missed");
      usleep((thisSlotBegin + cycleDuration - gpsTime) * 1000);
    } else {
      //we're in our slot...  send as much data as we can
      LOG_TRACE("In slot... " << thisSlotEnd - gpsTime << " remaining in slot");
      bool slotTimeAvailable = true;

      while(slotTimeAvailable && (!sentPliRelayThisCycle || receiver->isMessageAvailable())) {
        uint8_t slotIndex = 0;
        std::string relayMessage = "";
        int nextMessageLength = 0;
        if(!sentPliRelayThisCycle) {
          relayMessage = receiver->getNextPliRelayPacket();
          nextMessageLength = relayMessage.length();
          if(relayMessage == "") {
            //no pli relay to be sent
            if(receiver->isMessageAvailable()) {
              nextMessageLength = receiver->getNextMessageSize() + sizeof(SerialHeader);
              sentPliRelayThisCycle = true;
            } else {
              break;
            }
          }
        } else {
          nextMessageLength = receiver->getNextMessageSize() + sizeof(SerialHeader);
        }

        if(nextMessageLength > maxPayloadSize) {
          //throw too-long messages out without processing
          LOG_WARN("Message of size " << nextMessageLength << " is greater than max payload size... dropping.");
          if(sentPliRelayThisCycle) {
            std::string *msg = receiver->getNextReceivedMessage();
            delete msg;
          }
        } else {
          //update GPS time; previous operation might have taken a while
          systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
          gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;

          int64_t timeLeft = thisSlotEnd - gpsTime;
          int64_t bytesThatWillFit = timeLeft * bytesPerMs;

          LOG_TRACE("Time: " << gpsTime << ", left in slot " << timeLeft << "ms, bytes sent " << thisSlotConsumed << "/" << maxPayloadSize);
          if(nextMessageLength <= (maxPayloadSize - thisSlotConsumed) && nextMessageLength <= bytesThatWillFit) {
            //send the message
            std::string *msg;
            if(!sentPliRelayThisCycle) {
              msg = &relayMessage;
            } else {
              msg = receiver->getNextReceivedMessage();
            }
            LOG_TRACE("Sending message, length " << msg->length() << " + header");
            sendMessage(msg, cycleDuration, slotNumber, slotIndex);
            slotIndex++;
            thisSlotConsumed += nextMessageLength;
            if(!sentPliRelayThisCycle) {
              //PLI relay messages are stack allocated, don't delete them
              sentPliRelayThisCycle = true;
            } else {
              delete msg;
            }
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
      //we shouldn't get here until the next slot begins, so reset our variable
      //tracking whether or not we've sent PLI relay
      pliCycleCount++;
      if(pliCycleCount == pliSendFrequency) {
        sentPliRelayThisCycle = false;
        pliCycleCount = 0;
      }
    }
  }
  return 0;
}

void SerialTransmitThread::sendMessage(std::string *msg, int cycleDuration, uint8_t slotNumber, uint8_t slotIndex) {
  //build the message header
  SerialHeader header;
  header.magic[0] = MAGIC[0];
  header.magic[1] = MAGIC[1];
  header.magic[2] = MAGIC[2];

  header.versionAndSlot = VERSION | slotNumber;
  header.size = msg->length();

  uint32_t fullChecksum = ACE::crc32(msg->data(), msg->length());
  header.payloadChecksum = static_cast<uint16_t>(fullChecksum);
  
  header.slotIndex = slotIndex;
  header.slotNumber = slotNumber;

  int64_t systemTime = ACE_OS::gettimeofday().get_msec(); //gets system time in milliseconds
  int64_t gpsTime = systemTime - gpsThread->getTimeDelta() / 1000 + gpsTimeOffset;
  header.hyperperiod = static_cast<uint16_t>(gpsTime / slotDuration);
  header.packetType = PACKETTYPE_NORMAL;
  header.reserved = 0;

  uint32_t headerChecksum = ACE::crc32(&header, sizeof(header) - sizeof(header.headerChecksum));
  header.headerChecksum = (uint16_t) headerChecksum;

  //send the header
  parent->write_string(std::string((char *)&header, sizeof(header)));
  //send the payload
  parent->write_string(*msg);
}

bool SerialTransmitThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}
