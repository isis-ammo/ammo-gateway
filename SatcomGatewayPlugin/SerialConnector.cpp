#include "SerialConnector.h"

#include "SatcomConfigurationManager.h"

SerialConnector::SerialConnector() :
closed(true),
serialDevice(),
serialConnector()
{
  
}

SerialConnector::~SerialConnector() {
  if(!isClosed()) {
    LOG_ERROR("SerialReaderThread: Entered destructor before stop()");
    stop();
    this->wait();
  }
}

int SerialConnector::svc() {
  LOG_ERROR("MAGIC NUMBER: " << std::hex << MAGIC_NUMBER << "::" << +MAGIC_NUMBER_BYTES[0] << " " << +MAGIC_NUMBER_BYTES[1] << " " << +MAGIC_NUMBER_BYTES[2] << " " << +MAGIC_NUMBER_BYTES[3])

  //bool status = connect();

  //if(status == false) {
  //  return 1;
  //}

  //return 0;
}

bool SerialConnector::connect() {
  const std::string listenPort = SatcomConfigurationManager::getInstance().getListenPort();

  //ACE-based serial initialization code
  int result = serialConnector.connect(serialDevice, ACE_DEV_Addr(listenPort.c_str()));
  if(result == -1) {
    LOG_ERROR("Couldn't open serial port " << listenPort << " (" << errno << ": " << strerror(errno) << ")" );
    return false;
  }
  
  ACE_TTY_IO::Serial_Params params;
  params.baudrate = SatcomConfigurationManager::getInstance().getBaudRate();
  params.xonlim = 0;
  params.xofflim = 0;
  params.readmincharacters = 0;
  #ifdef WIN32
  params.readtimeoutmsec = 1;
  #else
  params.readtimeoutmsec = -1; //negative value means infinite timeout
  #endif
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

  result = serialDevice.control(ACE_TTY_IO::SETPARAMS, &params);

  if(result == -1) {
    LOG_ERROR("Couldn't configure serial port");
    return false;
  }

  return true;
}

void SerialConnector::stop() {
  ThreadMutexGuard g(closeMutex);
  if(g.locked()) {
    closed = true;
  } else {
    LOG_ERROR("Error acquiring lock in SerialConnector::stop()")
  }
}

bool SerialConnector::isClosed() {
  volatile bool temp;
  {
    ThreadMutexGuard g(closeMutex);
    if(g.locked()) {
      temp = closed;
    } else {
      temp = false;
      LOG_ERROR("Error acquiring lock in SerialConnector::isClosed()")
    }
  }
  return temp;
}
