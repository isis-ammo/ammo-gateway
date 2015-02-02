/* Copyright (c) 2010-2015 Vanderbilt University
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef SERIAL_SERVICE_HANDLER_H
#define SERIAL_SERVICE_HANDLER_H

#include "GatewayConnector.h"
#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "ace/DEV_Connector.h"
#include "ace/TTY_IO.h"
#include "ace/OS_NS_unistd.h"
#include "protocol/AmmoMessages.pb.h"
#include "ace/Token.h"
#include <vector>
#include <queue>
#include <string>
#ifdef WIN32
  #include <windows.h>
#endif

#include <stdint.h>

class SerialMessageProcessor;
class SerialTransmitThread;
class GatewayReceiver;
class GpsThread;

const unsigned int HEADER_MAGIC_NUMBER = 0xfeedbeef;
/**
 Must remain compatible with AmmoCore/src::edu.vu.isis.ammo.core.network::AmmoGatewayMessage.java
*/

struct MessageHeader {
  unsigned int magicNumber;    //Always set to 0xfeedbeef
  unsigned int size;           //size of the data (does *not* include header)
  char         priority;       //Message priority (larger numbers are higher priority, and will be processed first if messages are queued)
  char         error;          //Error code (if nonzero, size and message checksum should be zero)
  char         reserved[2];    //Reserved for future use, and to ensure header word alignment
  unsigned int checksum;       //CRC32 checksum of the data (does *not* include header)
  unsigned int headerChecksum; //CRC32 checksum of the header, less this checksum.  Does *not* include data, or itself.
};

struct SerialHeader {
  uint8_t  magic[3];
  uint8_t  versionAndSlot; //first two bits set to 01 and next six bits encode phone ID
  uint16_t size;            //size of the data (not including this header)
  uint16_t payloadChecksum; //low two bytes of CRC32 checksum of data (doesn't include header)
  uint8_t  slotIndex;       //the index of this packet within its slot
  uint8_t  slotNumber;      //the slot number of the sending device
  uint16_t hyperperiod;     //current GPS time / cycle duration; low order 16 bits
  uint8_t  packetType;      //The packet type (for resend functionality)
  uint8_t  reserved;        //Reserved for future use
  uint16_t headerChecksum;  //low two bytes of CRC32 checksum of header
};

//Valid values for SerialHeader::packetType
const uint16_t PACKETTYPE_NORMAL = 0x0001;
const uint16_t PACKETTYPE_RESEND = 0x0002;
const uint16_t PACKETTYPE_ACK    = 0x0003;
const uint16_t PACKETTYPE_RELAY  = 0x0004;

//error values for MessageHeader
const char SS_NO_ERROR = 0;
const char INVALID_MAGIC_NUMBER = 1;
const char INVALID_HEADER_CHECKSUM = 2;
const char INVALID_MESSAGE_CHECKSUM = 3;
const char MESSAGE_TOO_LARGE = 4;

struct QueuedMessage {
  char priority;
  unsigned long long messageCount; //messages of the same priority should come out in first-in first-out order (STL queue doesn't guarantee this)
  ammo::protocol::MessageWrapper *message;
};

class QueuedMessageComparison {
public:
  bool operator()(QueuedMessage &first, QueuedMessage &second) { //returns true if first is lower priority than second
    if(first.priority < second.priority) {
      return true;
    } else if(first.priority == second.priority && first.messageCount > second.messageCount) {
      return true;
    } else {
      return false;
    }
  }
};

class SerialServiceHandler {
public:
  SerialServiceHandler(GpsThread *gpsThread);
  
  int open(void *ptr = 0);

  void receiveData();		/* blocking call - would not terminate */
  
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum, char priority);
  
  void sendMessage(ammo::protocol::MessageWrapper *msg, char priority);
  ammo::protocol::MessageWrapper *getNextMessageToSend();
  
  ammo::protocol::MessageWrapper *getNextReceivedMessage();
  void addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority);
  
  int write_a_char(unsigned char toWrite);
  int write_string(const std::string &toWrite);

  ~SerialServiceHandler();
  
protected:
  std::string name;  // COM port name - used for logging
  ACE_TTY_IO serialDev;
  ACE_DEV_Connector serialConnector;

  unsigned char read_a_char();


  void sendErrorPacket(char errorCode);
  
  typedef enum {
    READING_HEADER = 0,
    READING_DATA = 1
  } ReaderState;
  
  ReaderState state;
  MessageHeader messageHeader;
  char *collectedData;
  unsigned int position;
  
  char *dataToSend;
  unsigned int sendPosition;
  unsigned int sendBufferSize;
  
  bool connectionClosing;
  
  std::string deviceId; //not validated; just for pretty logging
  
  SerialMessageProcessor *messageProcessor;
  ACE_Thread_Mutex sendQueueMutex;
  ACE_Thread_Mutex receiveQueueMutex;
  
  GatewayReceiver *receiver;
  SerialTransmitThread *transmitThread;

  ACE_Token serialPortToken;

  typedef std::priority_queue<QueuedMessage, std::vector<QueuedMessage>, QueuedMessageComparison> MessageQueue;
  MessageQueue sendQueue;
  MessageQueue receiveQueue;
  
  unsigned long long sentMessageCount;
  unsigned long long receivedMessageCount;
};

#endif        //  #ifndef SERIAL_SERVICE_HANDLER_H

