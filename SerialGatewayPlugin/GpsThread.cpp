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
#include <stdint.h>
#include <sstream>

#include "SerialConfigurationManager.h"

using namespace std;

GpsThread::GpsThread() : closeMutex(), closed(false), deltaHistoryCount(0), timeDelta(0), latitude(0), longitude(0), fixAcquired(false) {
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
      /*
       * GPGGA message structure (essential fix data with time, 3D location, and accuracy):
       *
       * Example:  $GPGGA,185656.000,3608.6668,N,08647.6318,W,1,06,1.1,195.6,M,-31.7,M,,0000*60
       * Pos    Example      Definition
       * 0      $GPGGA       message type
       * 1      185656.000   time (HHMMSS.SSS)
       * 2      3608.6668    latitude (36 degrees, 8.6668 minutes)
       * 3      N            direction of latitude
       * 4      08647.6318   longitude (86 degrees, 47.6318 minutes)
       * 5      W            direction of longitude
       * 6      1            fix quality (0=invalid, 1=GPS fix, 2-8 are also valid and mean different fix types)
       * 7      06           number of satellites being tracked
       * 8      1.1          horizontal "dilution" of precision (measurement of accuracy; <1 is ideal, 1-2 is excellent, 2-5 is good)
       * 9,10   195.6,M      altitude above mean sea level (in meters)
       * 11,12  -31.7,M      height of mean sea level above WGS84 ellipsoid
       * 13     empty field  time since last DGPS update
       * 14     0000         DGPS station ID
       * XX     *60          checksum
       */
      // Divide the returned GPS string into tokens (separated by ',')
      char separator = ',';
      std::vector<std::string> tokenizedMsg;

      std::string::size_type pos1 = 0;
      std::string::size_type pos2 = 0;
      while (pos2 != std::string::npos)
        {
          pos2 = msg.find_first_of(separator, pos1);
          std::string t;
          if (pos2 != std::string::npos)
            {
              t = msg.substr(pos1, pos2-pos1);
              pos1 = (pos2 + 1);
            }
          else
            {
              t = msg.substr(pos1);
            }
          tokenizedMsg.push_back(t);
        }

      if(tokenizedMsg.size() == 15) { //GPGGA message should have 15 elements
        string time = tokenizedMsg[1];
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
        int64_t dMicroseconds = timeinfo->tm_sec * 1e6 + msgTime.usec() - (static_cast<int>(seconds * 1e6));
        //LOG_INFO("dHours: " << dHours << " dMinutes: " << dMinutes << " dMicroseconds: " << dMicroseconds);
        
        //total time delta in microseconds
        int64_t delta = static_cast<int64_t>(((dHours*60 + dMinutes)*60))*1e6 + dMicroseconds;
        //LOG_INFO("  Delta: " << delta);
        
        deltaHistory[deltaHistoryCount % DELTA_HISTORY_SAMPLES] = delta;
        deltaHistoryCount++;
        
        //do a rolling average (FIR filter) to remove some of the jitter
        int64_t deltaTotal = 0;
        for(int i = 0; i < min(deltaHistoryCount, DELTA_HISTORY_SAMPLES); i++) {
          deltaTotal += deltaHistory[i];
        }
        
        int64_t deltaAverage = deltaTotal / min(deltaHistoryCount, DELTA_HISTORY_SAMPLES);
        //LOG_INFO("Using average delta " << deltaAverage << " (" << min(deltaHistoryCount, DELTA_HISTORY_SAMPLES) << " samples)");
        setTimeDelta(deltaAverage);

        //get latitude and longitude if fix is good enough
        string latString = tokenizedMsg[2];
        string latDirString = tokenizedMsg[3];
        string lonString = tokenizedMsg[4];
        string lonDirString = tokenizedMsg[5];
        string fixType = tokenizedMsg[6];
        if(fixType != "0" && latString != "" && latDirString != "" && lonString != "" && lonDirString != "") {
          string latDegreesString = latString.substr(0, 2);
          string latMinutesString = latString.substr(2, string::npos);
          string lonDegreesString = lonString.substr(0, 3);
          string lonMinutesString = lonString.substr(3, string::npos);
          
          int latDegrees = atoi(latDegreesString.c_str());
          double latMinutes = atof(latMinutesString.c_str());
          int lonDegrees = atoi(lonDegreesString.c_str());
          double lonMinutes = atof(lonMinutesString.c_str());
          
          double tempLatitude = (double) latDegrees + latMinutes / 60.0;
          double tempLongitude = (double) lonDegrees + lonMinutes / 60.0;
          
          if(latDirString == "S") {
            tempLatitude = tempLatitude * -1;
          }
          
          if(lonDirString == "W") {
            tempLongitude = tempLongitude * -1;
          }
          
          //LOG_INFO("Location: Lat: " << tempLatitude << " Lon:" << tempLongitude);
          setPosition(true, tempLatitude, tempLongitude);
        } else {
          //LOG_INFO("Inadequate GPS fix");
          setPosition(false, 0, 0);
        }
      } else {
        LOG_WARN("Malformed GGA message");
      }
    }
  }
  
  return true;
}

void GpsThread::setTimeDelta(int64_t delta) {
  timeDeltaMutex.acquire();
  timeDelta = delta;
  timeDeltaMutex.release();
}

int64_t GpsThread::getTimeDelta() {
  volatile int64_t temp;
  timeDeltaMutex.acquire();
  temp = timeDelta;
  timeDeltaMutex.release();
  return temp;
}

void GpsThread::setPosition(bool fix, double lat, double lon) {
  timeDeltaMutex.acquire();
  fixAcquired = fix;
  latitude = lat;
  longitude = lon;
  timeDeltaMutex.release();
}

bool GpsThread::getPosition(double &lat, double &lon) {
  timeDeltaMutex.acquire();
  lat = latitude;
  lon = longitude;
  bool status = fixAcquired;
  timeDeltaMutex.release();
  return status;
}

bool GpsThread::initSerial() {
  // Open the serial port.
  string gpsPort = SerialConfigurationManager::getInstance()->getGpsPort();
  
  //ACE-based serial initialization code
  int result = serialConnector.connect(serialDev, ACE_DEV_Addr(gpsPort.c_str()));
  if(result == -1) {
    LOG_ERROR("Couldn't open serial port " << gpsPort);
    exit(-1);
  }
  
  ACE_TTY_IO::Serial_Params params;
  params.baudrate = 4800;
  params.xonlim = 0;
  params.xofflim = 0;
  params.readmincharacters = 0;
  params.readtimeoutmsec = -1; //negative value means infinite timeout
  params.paritymode = "NONE";
  params.ctsenb = false;
  params.rtsenb = 0;
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
    LOG_ERROR("Couldn't configure serial port");
    exit(-1);
  }

  return true;
}

char GpsThread::read_a_char() {
  unsigned char temp;
    
    ssize_t count = 0;
    while(count == 0) {
      count = serialDev.recv_n((void *) &temp, 1);
    }

    if ( count == -1 )
    {
      LOG_ERROR( "Read returned -1" );
      exit( -1 );
    }
    else if ( count >= 1 )
    {

    }
    else if ( count == 0 )
    {
      LOG_ERROR( "Read returned 0" );
      exit( -1 );
    }
    return temp;
}

int GpsThread::write_a_char(unsigned char toWrite) {
  ssize_t bytesWritten = 0;
  while(bytesWritten == 0) {
    bytesWritten = serialDev.send_n(&toWrite, 1);
  }
  
  if ( bytesWritten == -1 )
  {
    LOG_ERROR( "Write returned -1" );
    exit( -1 );
  }
  else if ( bytesWritten >= 1 )
  {
    return bytesWritten;
  }
  else if ( bytesWritten == 0 )
  {
    LOG_ERROR( "Write returned 0" );
    exit( -1 );
  }
  return bytesWritten;
}
