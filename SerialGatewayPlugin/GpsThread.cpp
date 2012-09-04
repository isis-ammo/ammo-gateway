#include "GpsThread.h"

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
#include <sstream>

#include "SerialConfigurationManager.h"

using namespace std;

GpsThread::GpsThread() : closeMutex(), closed(false), deltaHistoryCount(0), timeDelta(0) {
#ifdef WIN32
  hComm = NULL;
#else
  gFD = NULL;
#endif
}

GpsThread::~GpsThread() {
  
}

int GpsThread::svc() {
  bool success = initSerial();
  
  if(success) {
    ostringstream *message = new ostringstream();
    ACE_Time_Value messageTime = ACE_OS::gettimeofday();
    LOG_DEBUG("Waiting for data");
    while(!isClosed()) {
      //NMEA-formatted data consists of sentences terminated by \r\n
      char readChar = read_a_char();
      
      if(readChar == '\r') {
        //ignore
      } else if(readChar == '\n') {
        //we've read a full message, process it
        processMessage(messageTime, message->str());
        delete message;
        message = new ostringstream();
        messageTime = ACE_OS::gettimeofday();
      } else {
        //if a $ (beginning of message), get the time of the message
        if(readChar == '$') {
          messageTime = ACE_OS::gettimeofday();
        }
        //put the char in the message
        *message << readChar;
      }
    }
  } else {
    LOG_ERROR("GPS serial startup failed...  no time sync.");
  }
  
  return 0;
}

void GpsThread::stop() {
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
}

bool GpsThread::isClosed() {
  volatile bool temp;
  closeMutex.acquire();
  temp = closed;
  closeMutex.release();
  return temp;
}

bool GpsThread::processMessage(ACE_Time_Value msgTime, std::string msg) {
  //LOG_INFO("At: " << msgTime << " GPS: " << msg);
  size_t firstCommaPosition = msg.find(",");
  
  time_t sec = msgTime.sec();
  tm *timeinfo = ACE_OS::gmtime(&sec);
  
  if(firstCommaPosition != string::npos) {
    string sentenceType = msg.substr(0, firstCommaPosition);
    if(sentenceType == "$GPGGA") {
      size_t secondCommaPosition = msg.find(",", firstCommaPosition + 1);
      if(secondCommaPosition != string::npos) {
        string time = msg.substr(firstCommaPosition + 1, secondCommaPosition - firstCommaPosition - 1);
        //LOG_INFO("GPGGA: " << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << ":" << msgTime.usec() << " :: " << time);
        
        string hoursString = time.substr(0, 2);
        string minutesString = time.substr(2,2);
        string secondsString = time.substr(4, string::npos);
        
        int hours = atoi(hoursString.c_str());
        int minutes = atoi(minutesString.c_str());
        double seconds = atof(secondsString.c_str());
        //LOG_INFO("XX GPS TIME: " << hoursString << ":" << minutesString << ":" << secondsString);
        
        //calculate delta
        int dHours = timeinfo->tm_hour - hours;
        int dMinutes = timeinfo->tm_min - minutes;
        long dMicroseconds = timeinfo->tm_sec * 1e6 + msgTime.usec() - (static_cast<int>(seconds * 1e6));
        //LOG_INFO("dHours: " << dHours << " dMinutes: " << dMinutes << " dMicroseconds: " << dMicroseconds);
        
        //total time delta in microseconds
        long delta = static_cast<long>(((dHours*60 + dMinutes)*60))*1e6 + dMicroseconds;
        //LOG_INFO("  Delta: " << delta);
        
        deltaHistory[deltaHistoryCount % DELTA_HISTORY_SAMPLES] = delta;
        deltaHistoryCount++;
        
        //do a rolling average (FIR filter) to remove some of the jitter
        long deltaTotal = 0;
        for(int i = 0; i < min(deltaHistoryCount, DELTA_HISTORY_SAMPLES); i++) {
          deltaTotal += deltaHistory[i];
        }
        
        long deltaAverage = deltaTotal / min(deltaHistoryCount, DELTA_HISTORY_SAMPLES);
        //LOG_INFO("Using average delta " << deltaAverage << " (" << min(deltaHistoryCount, DELTA_HISTORY_SAMPLES) << " samples)");
        setTimeDelta(deltaAverage);
      } else {
        LOG_WARN("Malformed GGA message");
      }
    }
  }
  
  return true;
}

void GpsThread::setTimeDelta(long delta) {
  timeDeltaMutex.acquire();
  timeDelta = delta;
  timeDeltaMutex.release();
}

long GpsThread::getTimeDelta() {
  volatile long temp;
  timeDeltaMutex.acquire();
  temp = timeDelta;
  timeDeltaMutex.release();
  return temp;
}

bool GpsThread::initSerial() {
  // Open the serial port.
  string gpsPort = SerialConfigurationManager::getInstance()->getGpsPort();
  
#ifdef WIN32
  this->hComm = CreateFile( gpsPort.c_str(),
                           GENERIC_READ | GENERIC_WRITE,
                           0,
						   0,
						   OPEN_EXISTING,
						   0,
						   0);
  if (this->hComm == INVALID_HANDLE_VALUE) {
    int err = GetLastError();
    LOG_ERROR("GPS open "<< gpsPort << " error code: " << err );
    return false;
  }
#else
  LOG_DEBUG("Opening " << gpsPort);
  // Open the port
  gFD = ::open( gpsPort.c_str(), O_RDWR | O_NOCTTY );// | O_NONBLOCK );
  if ( gFD == -1 )
  {
    LOG_ERROR("GPS open "<< gpsPort << ": error: " << strerror(errno));
    return false;
  }
#endif
  
  // Configure the serial port.
  LOG_DEBUG("Configuring");
#ifdef WIN32
  DCB dcb;

  FillMemory(&dcb, sizeof(dcb), 0);
  dcb.DCBlength = sizeof(dcb);

  if (!BuildCommDCB("4800,n,8,1", &dcb)) {   
    LOG_ERROR("GPS could not build dcb: error " << GetLastError());
    return false;
  }

  if (!SetCommState(this->hComm, &dcb)) {
    LOG_ERROR("GPS could not set comm state: error " << GetLastError());
    CloseHandle(this->hComm);
    return false;
  }
#else
  // Get the attributes for the port
  // struct termios config;
  // int result = tcgetattr( gFD, &config );
  // if ( result == -1 )
  // {
  //     perror( "tcgetattr" );
  //     exit( -1 );
  // }
  
  // // Set baud rate and 8, NONE, 1
  
  // // SETTING KEY:
  // // 1 -- ignore BREAK condition
  // // 2 -- map BREAK to SIGINTR
  // // 3 -- mark parity and framing errors
  // // 4 -- strip the 8th bit off chars
  // // 5 -- map NL to CR
  // // 6 -- ignore CR
  // // 7 -- map CR to NL
  // // 8 -- enable output flow control (software flow control)
  // // 9 -- enable input flow control (software flow control)
  // // 10-- any char will restart after stop (software flow control)
  // // 11-- postprocess output (not set = raw output)
  // // 12-- enable echoing of input characters
  // // 13-- echo NL
  // // 14-- enable canonical input (else raw)
  // // 15-- enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
  // // 16-- enable extended functions
  // // 17-- parity enable
  // // 18-- send 2 stop bits
  // // 19-- character size mask
  // // 20-- 8 bits
  // // 21-- enable follwing output processing
  
  // //		               1      2      3      4     5      6     7    8     9    10
  // config.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY);
  // //                  11
  // config.c_oflag &= ~OPOST;
  // //                   12    13     14    15    16
  // config.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  // //		              17     18     19
  // //                 20
  // config.c_cflag |= CS8;
  // //                   21
  // config.c_cflag |= CRTSCTS;
  
  // config.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON|IXOFF|IXANY);
  // config.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
  // config.c_cflag &= ~(PARENB|CSTOPB|CSIZE);
  // config.c_cflag |= CS8;
  
  // 	speed_t speed = B9600;
  
  // cfsetispeed(&config, speed);
  // cfsetospeed(&config, speed);
  
  // bzero(&config, sizeof(config) );
  // config.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
  // config.c_iflag = IGNPAR | ICRNL;
  // config.c_oflag = 0;
  // //config.c_lflag = ICANON;
  // config.c_cc[VMIN] = 1;	// blocking read until 1 character arrives
  
  struct termios cfg;
  
  if (tcgetattr(gFD, &cfg))
  {
    close(gFD);
    // TODO: throw an exception
    return false;
  }
  
  // Set baud rate
  cfsetispeed( &cfg, B4800 );
  cfsetospeed( &cfg, B4800 );
  
  cfmakeraw( &cfg );
  
  // Always set these
  cfg.c_cflag |= (CLOCAL | CREAD);
  
  // Set 8, None, 1
  cfg.c_cflag &= ~PARENB;
  cfg.c_cflag &= ~CSTOPB;
  cfg.c_cflag &= ~CSIZE;
  cfg.c_cflag |= CS8;
  
  // Enable hardware flow control
  cfg.c_cflag |= CRTSCTS;
  
  // Use raw input rather than canonical (line-oriented)
  cfg.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  
  // Disable software flow control
  cfg.c_iflag &= ~(IXON | IXOFF | IXANY);
  
  // Use raw output rather than processed (line-oriented)
  cfg.c_oflag &= ~OPOST;
  
  // Read one character at a time.  VTIME defaults to zero, so reads will
  // block indefinitely.
  cfg.c_cc[VMIN] = 1;
  
  // Other "c" bits
  //cfg.c_iflag |= IGNBRK; // Ignore break condition
  // The constant IUCLC is not present on OSX, so we only use it on non-OSX
  // platforms (where __APPLE__ is not defined).
  #ifndef __APPLE__
  cfg.c_iflag &= ~( IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IUCLC );
  #else 
  cfg.c_iflag &= ~( IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL );
  #endif
  
  // Other "l" bits
  cfg.c_lflag &= ~IEXTEN;
  
  
  // Old, bad code. Sort of works, but was using canonical mode, which
  // we don't want.
  
  //struct termios config;
  //memset( &config, 0, sizeof(config) );
  //config.c_cflag = B9600 | CRTSCTS  | CS8 | CLOCAL | CREAD;
  //config.c_iflag = IGNPAR | ICRNL;
  //config.c_oflag = 0;
  //config.c_cc[VMIN] = 1;
  
  tcflush( gFD, TCIFLUSH );
  
  if (tcsetattr(gFD, TCSANOW, &cfg))
  {
    close(gFD);
    /* TODO: throw an exception */
    return false;
  }
  
  // tcflush( gFD, TCIFLUSH );
  // tcsetattr( gFD, TCSANOW, &config );
#endif
  return true;
}

char GpsThread::read_a_char() {
  unsigned char temp;
  
  while ( true )
  {
    // printf( "about to read()..." );
#ifdef WIN32
    DWORD count = 0;
    
	while (count == 0) {
      if (!ReadFile(this->hComm, &temp, 1, &count, NULL)) {
        int err = GetLastError();
        LOG_ERROR("ReadFile failed with error code: " << err);
	    exit(-1);
      }
	  Sleep(1);
	}
#else
    ssize_t count = read( gFD, &temp, 1 );
#endif

    if ( count == -1 )
    {
      LOG_ERROR( "Read returned -1" );
      exit( -1 );
    }
    else if ( count >= 1 )
    {
      break;
    }
    else if ( count == 0 )
    {
      LOG_ERROR( "Read returned 0" );
      exit( -1 );
    }
  }
  return temp;
}

int GpsThread::write_a_char(unsigned char toWrite) {
  #ifdef WIN32
  DWORD ret = 0;
  if (!WriteFile(this->hComm, &toWrite, sizeof(toWrite), &ret, NULL)) {
    int err = GetLastError();
    LOG_ERROR("ReadFile failed with error code: " << err);
    exit(-1);
  }
  #else
  ssize_t ret = write(gFD, &toWrite, sizeof(toWrite));
  #endif
  
  if ( ret == -1 )
  {
    LOG_ERROR( "Read returned -1" );
    exit( -1 );
  }
  else if ( ret >= 1 )
  {
    return ret;
  }
  else if ( ret == 0 )
  {
    LOG_ERROR( "Read returned 0" );
    exit( -1 );
  }
  return ret;
}
