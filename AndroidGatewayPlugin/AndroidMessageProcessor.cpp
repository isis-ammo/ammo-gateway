#include "AndroidMessageProcessor.h"
#include "AndroidServiceHandler.h"

#include "GWSecurityMgr.h"

#include "log.h"
#include <fstream>

using namespace ammo::gateway;

AndroidMessageProcessor::AndroidMessageProcessor(AndroidServiceHandler *serviceHandler) :
closed(false),
closeMutex(),
newMessageMutex(),
newMessageAvailable(newMessageMutex),
commsHandler(serviceHandler),
gatewayConnector(NULL),
deviceId(""),
deviceIdAuthenticated(false),
secP_(NULL) 
{
  //need to initialize GatewayConnector in the main thread; the constructor always
  //happens in the main thread
  gatewayConnector = new GatewayConnector(this);

  // create the Security Manager ...
  secP_ = new GWSecurityMgr("GW01");// right now hard coding ... later get it properly 
}

AndroidMessageProcessor::~AndroidMessageProcessor() {
  LOG_TRACE(commsHandler << " In ~AndroidMessageProcessor()");
  if(gatewayConnector) {
    delete gatewayConnector;
  }
  
  if(secP_) {
    delete secP_;
  }

}

int AndroidMessageProcessor::open(void *args) {
  closed = false;
  return 0;
}

int AndroidMessageProcessor::close(unsigned long flags) {
  LOG_TRACE(commsHandler << " Closing MessageProcessor (in AndroidMessageProcessor.close())");
  closeMutex.acquire();
  closed = true;
  closeMutex.release();
  
  signalNewMessageAvailable();
  return 0;
}

bool AndroidMessageProcessor::isClosed() {
  volatile bool ret; //does this need to be volatile to keep the compiler from optimizing it out?

  closeMutex.acquire();
  ret = closed;
  closeMutex.release();
  
  return ret;
}

int AndroidMessageProcessor::svc() {
  while(!isClosed()) {    
    ammo::protocol::MessageWrapper *msg = NULL;
    
    do {
      msg = commsHandler->getNextReceivedMessage();
      if(msg) {
        processMessage(*msg);
        delete msg;
      }
    } while (msg != NULL);
    if(!isClosed()) {
      newMessageMutex.acquire();
      newMessageAvailable.wait();
      newMessageMutex.release();
    }
  }
  return 0;
}

void AndroidMessageProcessor::signalNewMessageAvailable() {
  newMessageMutex.acquire();
  newMessageAvailable.signal();
  newMessageMutex.release();
}

void AndroidMessageProcessor::processMessage(ammo::protocol::MessageWrapper &msg) {
  LOG_TRACE(commsHandler << " Message Received: " << msg.DebugString());
  
  if(msg.type() == ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Authentication Message...");

    LOG_DEBUG(commsHandler << " Calling Authenticate...");
    
    ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();
    
    ammo::protocol::MessageWrapper *outMsg = new ammo::protocol::MessageWrapper();

    if (authMessage.type() == ammo::protocol::AuthenticationMessage_Type_CLIENT_NONCE)
    {
      secP_->set_client_nonce (authMessage.message());

      LOG_TRACE(commsHandler << "Got the Client Nonce ");

      std::vector<unsigned char> nonce = secP_->get_Server_Nonce ();

      secP_->set_device_id(authMessage.device_id());

      ammo::protocol::AuthenticationMessage *authRes = outMsg->mutable_authentication_message();
      outMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE);

      authRes->set_result(ammo::protocol::AuthenticationMessage_Status_SUCCESS);
      authRes->set_type(ammo::protocol::AuthenticationMessage_Type_SERVER_NONCE);

      std::string sigStr; 
      sigStr.assign (nonce.begin (), nonce.end ());

      ofstream out("serverNonce");
      out.write(sigStr.c_str(), sigStr.length());
      out.close ();

      LOG_DEBUG(commsHandler << "Server Nonce Length is " << sigStr.length());

      authRes->set_message(sigStr);
      
      LOG_DEBUG(commsHandler << " Returning from Authenticate...");
      commsHandler->sendMessage(outMsg);


    } else if (authMessage.type() == ammo::protocol::AuthenticationMessage_Type_CLIENT_KEYXCHANGE)
    {
      
      string keyXChange = authMessage.message();

      LOG_TRACE(commsHandler << "Got the Key Exchange");

      secP_->set_keyXchange (authMessage.message());

    } else if (authMessage.type() == ammo::protocol::AuthenticationMessage_Type_CLIENT_PHNAUTH)
    {
      
      string phnAuth = authMessage.message();

      LOG_TRACE(commsHandler << "Got the Phone Auth");

      secP_->set_phn_auth (phnAuth);

      bool ver = secP_->verify_phone_auth ();

      if (ver)
        secP_->generate_master_secret ();


      LOG_TRACE(commsHandler << "Verified " << ((ver)? "OK": "FAILED"));

    } else if (authMessage.type() == ammo::protocol::AuthenticationMessage_Type_CLIENT_FINISH) 
    {
      
      LOG_TRACE(commsHandler << "Got the CLIENT FINISH MSG");

      string clnt_fin = authMessage.message();

      if(secP_->verify_client_finish (clnt_fin))
      {
        LOG_TRACE(commsHandler << "Client Finish Verified OK"); 

        // Client Finish verified ok ... so now send the gateway finish ...
        ammo::protocol::AuthenticationMessage *authRes = outMsg->mutable_authentication_message();
        outMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_MESSAGE);

        authRes->set_result(ammo::protocol::AuthenticationMessage_Status_SUCCESS);
        authRes->set_type(ammo::protocol::AuthenticationMessage_Type_SERVER_FINISH);

        authRes->set_message(secP_->get_server_finish ());

        LOG_DEBUG(commsHandler << " Returning from Authenticate after sending Gateway Finish...");
        commsHandler->sendMessage(outMsg);
      }else {
        LOG_TRACE(commsHandler << "Client Finish Verified False"); 
        // Client Finish verified false ... so now send, auth false ...
      }
    }

/*
    if (secP->Authenticate (msg))
    {
      // send success ...with its own sign ....

      std::vector<unsigned char> sig = secP->get_gateway_sign ();

      printf ("\n Got the sign .. the sign length is %d\n", sig.size());
      
      std::string sigStr; 
      sigStr.assign (sig.begin (), sig.end ());

      authRes->set_message(sigStr);
      
      printf ("\n The sign is %s, and its length is %d\n", sigStr.c_str(), sigStr.size ());

      //send its own sign 

      // If Authenticated, let Gateway know about the new device ....
      if(gatewayConnector != NULL) {
        ammo::protocol::AuthenticationMessage authMessage = msg.authentication_message();

        gatewayConnector->associateDevice(authMessage.device_id(), 
                                          authMessage.user_id(), 
                                          authMessage.user_key());
      }

      this->deviceId = authMessage.device_id();
    }
    else
    {
      // send failed 
      authRes->set_result(ammo::protocol::AuthenticationResult_Status_FAILED);
    }
    */
    

    // Send back Authenticated result to Gateway ...


  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Data Message...");
    if(gatewayConnector != NULL) {
      ammo::protocol::DataMessage dataMessage = msg.data_message();
      MessageScope scope;
      if(dataMessage.scope() == ammo::protocol::LOCAL) {
        scope = SCOPE_LOCAL;
      } else {
        scope = SCOPE_GLOBAL;
      }
      
      PushData pushData;
      pushData.uri = dataMessage.uri();
      pushData.mimeType = dataMessage.mime_type();
      pushData.data = dataMessage.data();
      pushData.scope = scope;
      gatewayConnector->pushData(pushData);
      ammo::protocol::MessageWrapper *ackMsg = new ammo::protocol::MessageWrapper();
      ammo::protocol::PushAcknowledgement *ack = ackMsg->mutable_push_acknowledgement();
      ack->set_uri(dataMessage.uri());
      ackMsg->set_type(ammo::protocol::MessageWrapper_MessageType_PUSH_ACKNOWLEDGEMENT);
      LOG_DEBUG(commsHandler << " Sending push acknowledgment to connected device...");
      commsHandler->sendMessage(ackMsg);
      
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_SUBSCRIBE_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Subscribe Message...");
    MessageScope scope;
    if(msg.subscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::SubscribeMessage subscribeMessage = msg.subscribe_message();
      
      gatewayConnector->registerDataInterest(subscribeMessage.mime_type(), this, scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_UNSUBSCRIBE_MESSAGE) {
    LOG_DEBUG(commsHandler << " Received Unubscribe Message...");
    MessageScope scope;
    if(msg.unsubscribe_message().scope() == ammo::protocol::LOCAL) {
      scope = SCOPE_LOCAL;
    } else {
      scope = SCOPE_GLOBAL;
    }
    
    if(gatewayConnector != NULL) {
      ammo::protocol::UnsubscribeMessage unsubscribeMessage = msg.unsubscribe_message();
      
      gatewayConnector->unregisterDataInterest(unsubscribeMessage.mime_type(), scope);
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_PULL_REQUEST) {
    LOG_DEBUG(commsHandler << " Received Pull Request Message...");
    if(gatewayConnector != NULL && deviceIdAuthenticated) {
      ammo::protocol::PullRequest pullRequest = msg.pull_request();
      // register for pull response - 
      gatewayConnector->registerPullResponseInterest(pullRequest.mime_type(), this);
      // now send request
      PullRequest req;
      req.requestUid = pullRequest.request_uid();
      req.pluginId = this->deviceId;
      req.mimeType = pullRequest.mime_type();
      req.query = pullRequest.query();
      req.projection = pullRequest.projection();
      req.maxResults = pullRequest.max_results();
      req.startFromCount = pullRequest.start_from_count();
      req.liveQuery = pullRequest.live_query();
      gatewayConnector->pullRequest(req);
    } else {
      if(!deviceIdAuthenticated) {
        LOG_ERROR(commsHandler << " Attempted to send a pull request before authentication.");
      }
    }
  } else if(msg.type() == ammo::protocol::MessageWrapper_MessageType_HEARTBEAT) {
    LOG_DEBUG(commsHandler << " Received Heartbeat from device...");
    ammo::protocol::Heartbeat heartbeat = msg.heartbeat();
    
    ammo::protocol::MessageWrapper *heartbeatAck = new ammo::protocol::MessageWrapper();
    ammo::protocol::Heartbeat *ack = heartbeatAck->mutable_heartbeat();
    ack->set_sequence_number(heartbeat.sequence_number());
    heartbeatAck->set_type(ammo::protocol::MessageWrapper_MessageType_HEARTBEAT);
    
    LOG_DEBUG(commsHandler << " Sending heartbeat acknowledgement to connected device...");
    commsHandler->sendMessage(heartbeatAck);
  }
}

void AndroidMessageProcessor::onConnect(GatewayConnector *sender) {
}

void AndroidMessageProcessor::onDisconnect(GatewayConnector *sender) {
  
}

void AndroidMessageProcessor::onPushDataReceived(GatewayConnector *sender, ammo::gateway::PushData &pushData) {
  LOG_DEBUG(commsHandler << " Sending subscribed data to device...");
  LOG_DEBUG(commsHandler << "    " << pushData);
  
  std::string dataString(pushData.data.begin(), pushData.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper;
  ammo::protocol::DataMessage *dataMsg = msg->mutable_data_message();
  dataMsg->set_uri(pushData.uri);
  dataMsg->set_mime_type(pushData.mimeType);
  dataMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_DATA_MESSAGE);
  
  LOG_DEBUG(commsHandler << " Sending Data Push message to connected device");
  commsHandler->sendMessage(msg);
}

void AndroidMessageProcessor::onPullResponseReceived(GatewayConnector *sender, ammo::gateway::PullResponse &response) {
  LOG_DEBUG(commsHandler << " Sending pull response to device...");
  LOG_DEBUG(commsHandler << "    URI: " << response.uri << ", Type: " << response.mimeType);

  std::string dataString(response.data.begin(), response.data.end());
  ammo::protocol::MessageWrapper *msg = new ammo::protocol::MessageWrapper();
  ammo::protocol::PullResponse *pullMsg = msg->mutable_pull_response();

  pullMsg->set_request_uid(response.requestUid);
  pullMsg->set_plugin_id(response.pluginId);
  pullMsg->set_mime_type(response.mimeType);
  pullMsg->set_uri(response.uri);
  pullMsg->set_data(dataString);
  
  msg->set_type(ammo::protocol::MessageWrapper_MessageType_PULL_RESPONSE);
  
  LOG_DEBUG(commsHandler << " Sending Pull Response message to connected device");
  commsHandler->sendMessage(msg);
}



void AndroidMessageProcessor::onAuthenticationResponse(GatewayConnector *sender, bool result) {
  LOG_DEBUG(commsHandler << " Delegate: onAuthenticationResponse");
  if(result == true) {
    deviceIdAuthenticated = true;
  }
  
  ammo::protocol::MessageWrapper *newMsg = new ammo::protocol::MessageWrapper();
  newMsg->set_type(ammo::protocol::MessageWrapper_MessageType_AUTHENTICATION_RESULT);
  newMsg->mutable_authentication_result()->set_result(result ? ammo::protocol::AuthenticationResult_Status_SUCCESS : ammo::protocol::AuthenticationResult_Status_SUCCESS);
  commsHandler->sendMessage(newMsg);
}
