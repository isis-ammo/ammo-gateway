#include "SerialServiceHandler.h"
#include "SerialMessageProcessor.h"
#include "GatewayReceiver.h"
#include "GpsThread.h"
#include "SerialTransmitThread.h"
#include "SerialConfigurationManager.h"
#include "protocol/AmmoMessages.pb.h"

#ifdef WIN32
  #include <windows.h>
  #include <time.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#include <iostream>

#include "log.h"

const size_t MAX_PAYLOAD_SIZE = 1024;

using namespace std;

extern std::string gatewayAddress;
extern int gatewayPort;

#define PLI_TYPE "ammo/transapps.pli.locations"
#define CHAT_TYPE "ammo/transapps.chat.message_groupAll"

SerialServiceHandler::SerialServiceHandler(GpsThread *gpsThread) :
messageProcessor(NULL),
sendQueueMutex(), 
receiveQueueMutex(),
transmitThread(NULL)
{
  //constructor happens on main thread; both these objects need to be constructed here
  messageProcessor = new SerialMessageProcessor(this);
  if(gpsThread != NULL) {
    receiver = new GatewayReceiver();
    LOG_DEBUG(this->name << " - Creating serial transmit thread");
    transmitThread = new SerialTransmitThread(this, receiver, gpsThread);

    LOG_DEBUG(this->name << " - Registering interest in forwarded types")
    messageProcessor->gatewayConnector->registerDataInterest(PLI_TYPE, receiver, ammo::gateway::SCOPE_GLOBAL);
    messageProcessor->gatewayConnector->registerDataInterest(CHAT_TYPE, receiver, ammo::gateway::SCOPE_GLOBAL);
  }
}


int SerialServiceHandler::open(void *ptr)
{
  this->name = static_cast<char *>(ptr);

  //ACE-based serial initialization code
  int result = serialConnector.connect(serialDev, ACE_DEV_Addr(static_cast<char *>(ptr)));
  if(result == -1) {
    LOG_ERROR(this->name << " - Couldn't open serial port " << ptr);
    exit(-1);
  }
  
  ACE_TTY_IO::Serial_Params params;
  params.baudrate = SerialConfigurationManager::getInstance()->getBaudRate();
  params.xonlim = 0;
  params.xofflim = 0;
  params.readmincharacters = 0;
  params.readtimeoutmsec = 1; //negative value means infinite timeout
  params.paritymode = "NONE";
  params.ctsenb = true;
  params.rtsenb = 1;
  params.xinenb = false;
  params.xoutenb = false;
  params.modem = false;
  params.rcvenb = true;
  params.dsrenb = false;
  params.dtrdisable = false;
  params.databits = 8;
  params.stopbits = 1;

  result = serialDev.control(ACE_TTY_IO::SETPARAMS, &params);

  if(result == -1) {
    LOG_ERROR(this->name << " - Couldn't configure serial port");
    exit(-1);
  }
  
  // Configure internal service handler state.
  
  state = READING_HEADER;
  collectedData = NULL;
  position = 0;
  
  dataToSend = NULL;
  position = 0;
  
  messageHeader.magicNumber = 0;
  messageHeader.size = 0;
  messageHeader.checksum = 0;
  messageHeader.headerChecksum = 0;
  
  sentMessageCount = 0;
  receivedMessageCount = 0;
  
  connectionClosing = false;
  
  if(messageProcessor != NULL) {
    LOG_DEBUG(this->name << " - Starting message processor thread");
    messageProcessor->activate();
  }
  if(transmitThread != NULL) {
    LOG_DEBUG(this->name << " - Starting serial transmit thread");
    transmitThread->activate();
  }
  
  return 0;
}


//TODO: ACE provides gettimeofday; we shouldn't reimplement it
#ifdef WIN32
#define DELTA_EPOCH_IN_MICROSECONDS 11644473600000000ULL
int gettimeofday(struct timeval* tv, struct timezone* tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if (NULL != tv) {
    GetSystemTimeAsFileTime(&ft);

	tmpres |= ft.dwHighDateTime;
	tmpres <<= 32;
	tmpres |= ft.dwLowDateTime;

	tmpres -= DELTA_EPOCH_IN_MICROSECONDS;
	tmpres /= 10;
	tv->tv_sec = (int64_t) (tmpres / 1000000UL);
	tv->tv_usec = (int64_t) (tmpres % 1000000UL);
  }

  return 0;
}
#endif

void SerialServiceHandler::receiveData() {
  
  char phone_id = 127;
  unsigned short size = 0;
  int state = 0;
  unsigned char c = 0;
  char buf[MAX_PAYLOAD_SIZE] = { '\0' };
  struct timeval tv;

  write_a_char('\n');
  
  while ( true )
  {
    switch ( state )
    {
    case 0:
      // printf( "Waiting for magic.\n" ); fflush(stdout);
      c = read_a_char();
      if ( c == 0xef )
        state = c;
      break;
      
    case 0xef:
      c = read_a_char();
      if ( c == 0xbe || c == 0xef )
        state = c;
      else
        state = 0;
      break;
      
    case 0xbe:
      c = read_a_char();
      if ( c == 0xed )
        state = 1;
      else if ( c == 0xef )
        state = c;
      else
        state = 0;
      break;
      
    case 1:
      {
        for ( int i = 0; i < 13; ++i )
        {
          c = read_a_char();
          buf[i+3] = c;
        }
        phone_id = buf[3] & 0x3F;
        size = *(short *)&buf[4];
        
        state = 2;
      }
      break;
      
    case 2:
	    LOG_DEBUG(this->name << " - SLOT[" << (int) phone_id << "],Len[" << size << "]: ");
	    
	    if(size < MAX_PAYLOAD_SIZE - 16) {
        for (unsigned short i = 0; i < size; ++i)
	    {
	      c = read_a_char();
	      buf[i+16] = c;
	    }
	    {
	      int result = gettimeofday( &tv, NULL );
	      if ( result == -1 )
	      {
            LOG_ERROR(this->name << " - gettimeofday() failed\n" );
	        break;
	      }
	      
	      long ts = *(long *)&buf[8];
	      long rts = tv.tv_sec*1000 + tv.tv_usec / 1000; 
          LOG_DEBUG(this->name << " - Tdt(" << rts << "),Thh(" << ts << "),Tdel(" << rts - ts << ")");
	    }
	    
	    processData(&buf[16], size, *(short  *)&buf[6], 0); // process the message
	    } else {
	      LOG_ERROR(this->name << " - Received packet of invalid size: " << size);
	    }
	    state = 0;
	    break;
	    
	  default:
	    LOG_ERROR(this->name << " - SerialServiceHandler: unknown state");
	    break;
	  }
	}
	
	
}

int SerialServiceHandler::write_a_char(unsigned char toWrite) {
  ssize_t bytesWritten = 0;
  while(bytesWritten == 0) {
    serialPortToken.acquire();
    bytesWritten = serialDev.send_n(&toWrite, 1);
    serialPortToken.release();
  }
  
  if ( bytesWritten == -1 )
  {
    LOG_ERROR(this->name << " - Read returned -1" );
    LOG_ERROR(this->name << " - Write returned -1" );
    exit( -1 );
  }
  else if ( bytesWritten >= 1 )
  {
    return bytesWritten;
  }
  else if ( bytesWritten == 0 )
  {
    LOG_ERROR(this->name << " - Write returned 0" );
    exit( -1 );
  }
  return bytesWritten;
}

int SerialServiceHandler::write_string(const std::string &toWrite) {
  /*
   * Using ACE_Token here (rather than ACE_Thread_Mutex like most threaded code)
   * ensures that the sending thread gets an opportunity to send eventually (if
   * we just use a mutex, the sending thread ends up starved by the tight loop
   * in read_a_char()).
   */
  serialPortToken.acquire();
  ssize_t bytesWritten = serialDev.send_n(toWrite.data(), toWrite.length());
  serialPortToken.release();

  if(bytesWritten == -1) {
    LOG_ERROR(this->name << " - Write returned -1");
    exit(-1);
  } else if(bytesWritten != toWrite.length()) {
    LOG_WARN(this->name << "Didn't send entire message");
  }

  return bytesWritten;
}

unsigned char SerialServiceHandler::read_a_char()
{
  unsigned char temp;
  
  ssize_t count = 0;
  while(count == 0) {
    serialPortToken.acquire();
    ACE_Time_Value timeout(0, 10000); //10ms timeout; when this expires, we give the sender side an opportunity to send stuff
    count = serialDev.recv_n((void *) &temp, 1, &timeout);
    serialPortToken.release();
  }

  if ( count == -1 )
  {
    LOG_ERROR(this->name << " - Read returned -1" );
    exit( -1 );
  }
  else if ( count >= 1 )
  {

  }
  else if ( count == 0 )
  {
    LOG_ERROR(this->name << " - Read returned 0" );
    exit( -1 );
  }
  return temp;
}


int SerialServiceHandler::processData(char *data, unsigned int messageSize, unsigned int messageChecksum, char priority) {
  //Validate checksum
  unsigned int calculatedChecksum = ACE::crc32(data, messageSize);
  if( (calculatedChecksum & 0xffff) != (messageChecksum & 0xffff) ) {
    LOG_ERROR(this->name << " - " << (long) this << " Mismatched checksum " << std::hex << calculatedChecksum << " : " << messageChecksum);
    LOG_ERROR(this->name << " - " << (long) this << " size " << std::dec << messageSize ); // << " payload: " < );
    return -1;
  }
  
  //checksum is valid; parse the data
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  bool result = msg->ParseFromArray(data, messageSize);
  if(result == false) {
    LOG_ERROR(this->name << " - " << (long) this << " MessageWrapper could not be deserialized.");
    LOG_ERROR(this->name << " - " << (long) this << " Client must have sent something that isn't a protocol buffer (or the wrong type).");
    delete msg;
    return -1;
  }
  addReceivedMessage(msg, priority);
  messageProcessor->signalNewMessageAvailable();
  
  return 0;
}

void SerialServiceHandler::sendMessage(ammo::protocol::MessageWrapper *msg, char priority) {
}

void SerialServiceHandler::sendErrorPacket(char errorCode) {
}

ammo::protocol::MessageWrapper *SerialServiceHandler::getNextMessageToSend() {
  ammo::protocol::MessageWrapper *msg = NULL;
  return msg;
}

ammo::protocol::MessageWrapper *SerialServiceHandler::getNextReceivedMessage() {
  ammo::protocol::MessageWrapper *msg = NULL;
  receiveQueueMutex.acquire();
  if(!receiveQueue.empty()) {
    msg = receiveQueue.top().message;
    receiveQueue.pop();
  }
  receiveQueueMutex.release();
  
  return msg;
}

void SerialServiceHandler::addReceivedMessage(ammo::protocol::MessageWrapper *msg, char priority) {
  QueuedMessage queuedMsg;
  queuedMsg.priority = priority;
  queuedMsg.message = msg;
  
  if(priority != msg->message_priority()) {
    LOG_WARN(this->name << " - " << (long) this << " Priority mismatch on received message: Header = " << (int) priority << ", Message = " << msg->message_priority());
  }
  
  receiveQueueMutex.acquire();
  queuedMsg.messageCount = receivedMessageCount;
  receivedMessageCount++;
  receiveQueue.push(queuedMsg);
  receiveQueueMutex.release();
}

SerialServiceHandler::~SerialServiceHandler() {
  LOG_TRACE(this->name << " - " << (long) this << " In ~SerialServiceHandler");
  delete messageProcessor;
}
