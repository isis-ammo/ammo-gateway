#ifndef GATEWAYSECHANDLERH
#define GATEWAYSECHANDLERH

struct AuthMessage;

class GatewaySecHandler
{

  public: 

    virtual void sendMessage (AuthMessage& msg) = 0;

    virtual void authenticationComplete () = 0;

};

#endif
