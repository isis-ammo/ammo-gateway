#ifndef CROSS_GATEWAY_SERVICE_HANDLER_H
#define CROSS_GATEWAY_SERVICE_HANDLER_H

#include "ace/Svc_Handler.h"
#include "ace/SOCK_Stream.h"
#include "protocol/GatewayPrivateMessages.pb.h"
#include <vector>

class CrossGatewayServiceHandler : public ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> {
public:
  //CrossGatewayServiceHandler(ACE_Thread_Manager *tm, ACE_Message_Queue<ACE_NULL_SYNCH> *mq, ACE_Reactor *reactor);
  virtual ~CrossGatewayServiceHandler();
  int open(void *ptr = 0);
  int handle_input(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  int handle_output(ACE_HANDLE fd = ACE_INVALID_HANDLE);
  
  void sendData(ammo::gateway::protocol::GatewayWrapper &msg);
  int processData(char *collectedData, unsigned int dataSize, unsigned int checksum);
  
  bool sendSubscribeMessage(std::string mime_type);
  bool sendUnsubscribeMessage(std::string mime_type);
  
  bool sendPushedData(std::string uri, std::string mimeType, const std::string &data, std::string originUser);

  friend std::ostream& operator<< (std::ostream& out, const CrossGatewayServiceHandler& handler);
  friend std::ostream& operator<< (std::ostream& out, const CrossGatewayServiceHandler* handler);
  
protected:
  typedef ACE_Svc_Handler<ACE_SOCK_STREAM, ACE_NULL_SYNCH> super;
  
  typedef enum {
    READING_SIZE = 0,
    READING_CHECKSUM = 1,
    READING_DATA = 2
  } ReaderState;
  
  ReaderState state;
  unsigned int dataSize;
  unsigned int checksum;
  char *collectedData;
  unsigned int position;
  
  std::string username;
  bool usernameAuthenticated;
  
  std::vector<std::string> registeredHandlers;
  std::vector<std::string> registeredPullRequestHandlers;
};

#endif        //  #ifndef CROSS_GATEWAY_SERVICE_HANDLER_H

