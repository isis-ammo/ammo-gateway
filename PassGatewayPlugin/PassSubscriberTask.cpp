#include <iostream>

#include "log.h"

#include "PassSubscriberTask.h"
#include "PassConfigurationManager.h"
#include "soap/soapPASSSubscriberPortBindingService.h"
#include "soap/soapPASSPortBindingProxy.h"

PassSubscriberTask::PassSubscriberTask (void)
  : cfg_mgr_ (0),
    closed_ (false)
{
}

PassSubscriberTask::~PassSubscriberTask (void)
{
}

int PassSubscriberTask::open (void * /* args */)
{
  closed_ = false;
  return 0;
}

int
PassSubscriberTask::close (unsigned long /* flags */)
{
  LOG_DEBUG ("Stopping subscription server...");
  closed_ = true;
  return 0;
}

int
PassSubscriberTask::svc (void)
{
  LOG_DEBUG ("PassSubscriberTask::svc() started");
  cfg_mgr_ = PassConfigurationManager::getInstance ();
  PASSSubscriberPortBindingService service (SOAP_C_UTFSTRING);
  
  LOG_DEBUG ("Binding SOAP server to port 8054 on 0.0.0.0");
  
  SOAP_SOCKET res =
    service.bind (cfg_mgr_->getPassSubscriberInterface().c_str (), 
                  cfg_mgr_->getPassSubscriberPort (),
                  0);
                  
  if (res == SOAP_INVALID_SOCKET)
    {
      service.soap_print_fault (stderr);
      return 0;
    }
  
  int status = subscribe ();
  
  if (status != 0)
    {
      LOG_ERROR ("Error while subscribing...");
    }
    

  while (!closed_)
    {
      LOG_DEBUG ("Accepting connections...");
      
      // Server times out after 5 seconds (we ignore this error; we need
      // to time out so this thread can be cancelled).
      service.accept_timeout = 5;
      res = service.accept ();
      
      if (res == SOAP_INVALID_SOCKET && service.errnum != 0)
        {
          service.soap_print_fault (stderr);
          break;
        }
      else if (res == SOAP_INVALID_SOCKET && !service.errnum)
        {
          // TODO?
        }
      else
        {
          LOG_DEBUG ("Got a connection; serving it");
          status = service.serve ();
          
          if (status != SOAP_OK)
            {
              service.soap_print_fault (stderr);
            }
        }
    }

  LOG_DEBUG ("Unsubscribing...");
  status = unsubscribe ();
  
  if (status != 0)
    {
      LOG_ERROR ("Error while subscribing...");
    }
    
  service.soap_close_socket ();
  LOG_DEBUG ("Subscription thread terminating.");
  
  return 0;
}

int
PassSubscriberTask::subscribe (void)
{  
  PASSPortBindingProxy proxy;
  //proxy.cafile = "BccsServer.pem";
  proxy.ssl_flags = SOAP_SSL_SKIP_HOST_CHECK;
  
  proxy.soap_endpoint =
    cfg_mgr_->getPassServerAddress ().c_str ();
  
  _pass__subscribe subscriptionRequest;
  
  subscriptionRequest.return_USCOREaddress =
    cfg_mgr_->getPassSubscriberAddress ();
    
  subscriptionRequest.topic =
    cfg_mgr_->getPassTopic ();
    
  subscriptionRequest.subscriber_USCOREid =
    cfg_mgr_->getPassSubscriberId ();
  
  pass__status status;
  
  LOG_DEBUG ("Subscribing...");
  
  int result = proxy.subscribe (&subscriptionRequest, &status);
  
  if (result != SOAP_OK)
    {
      proxy.soap_print_fault (stderr);
      return 1;
    }
  else
    {
      LOG_DEBUG ("Subscribed: " << status.status_USCOREcode);
      
      if (status.status_USCOREphrase != 0)
        {
          LOG_DEBUG ("   (" << *(status.status_USCOREphrase) << ")");
        }
    }
  
  return 0;
}

int
PassSubscriberTask::unsubscribe (void)
{
  PASSPortBindingProxy proxy;
  //proxy.cafile = "BccsServer.pem";
  proxy.ssl_flags = SOAP_SSL_SKIP_HOST_CHECK;
  
  proxy.soap_endpoint =
    cfg_mgr_->getPassServerAddress ().c_str ();
  
  _pass__unsubscribe unsubscriptionRequest;
  
  unsubscriptionRequest.subscriber_USCOREid =
    cfg_mgr_->getPassSubscriberId ();
  
  pass__status status;
  
  LOG_DEBUG ("Unubscribing...");
  int result = proxy.unsubscribe (&unsubscriptionRequest, &status);
  
  if (result != SOAP_OK)
    {
      proxy.soap_print_fault (stderr);
      return 1;
    }
  else
    {
      LOG_DEBUG ("Unubscribed: " << status.status_USCOREcode);
      
      if (status.status_USCOREphrase != 0)
        {
          LOG_DEBUG ("   (" << *(status.status_USCOREphrase) << ")");
        }
    }
  
  return 0;
}

