#include "PassSubscriberTask.h"

#include <iostream>

#include "PassConfigurationManager.h"
#include "soap/soapPASSSubscriberPortBindingService.h"
#include "soap/soapPASSPortBindingProxy.h"

PassSubscriberTask::PassSubscriberTask() : closed(false) {
  
}

int PassSubscriberTask::open(void *args = 0) {
  closed = false;
  return 0;
}

int PassSubscriberTask::close(unsigned long flags = 0) {
  std::cout << "Stopping subscription server..." << std::endl << std::flush;
  closed = true;
  return 0;
}

int PassSubscriberTask::svc() {
  std::cout << "PassSubscriberTask::svc() started" << std::endl << std::flush;
  PASSSubscriberPortBindingService service(SOAP_C_UTFSTRING);
  
  std::cout << "Binding SOAP server to port 8054 on 0.0.0.0" << std::endl << std::flush;
  SOAP_SOCKET res = service.bind( PassConfigurationManager::getInstance()->getPassSubscriberInterface().c_str(),  PassConfigurationManager::getInstance()->getPassSubscriberPort(), 0);
  if(res == SOAP_INVALID_SOCKET) {
    service.soap_print_fault(stderr);
    return 0;
  }
  
  int status = subscribe();
  
  if(status != 0) {
    std::cout << "Error while subscribing..." << std::endl << std::flush;
  }
  
  while(!closed) {
    //std::cout << "Accepting connections..." << std::endl << std::flush;
    service.accept_timeout = 5; //server times out after 5 seconds (we ignore this error; we need to time out so this thread can be cancelled)
    res = service.accept();
    if(res == SOAP_INVALID_SOCKET && service.errnum) {
      service.soap_print_fault(stderr);
      break;
    } else if(res == SOAP_INVALID_SOCKET && !service.errnum) {
      
    } else {
      std::cout << "Got a connection; serving it" << std::endl << std::flush;
      status = service.serve();
      if(status != SOAP_OK) {
        service.soap_print_fault(stderr);
      }
    }
  }
  std::cout << "Unsubscribing..." << std::endl << std::flush;
  status = unsubscribe();
  if(status != 0) {
    std::cout << "Error while subscribing..." << std::endl << std::flush;
  }
  service.soap_close_socket();
  std::cout << "Subscription thread terminating." <<std::endl << std::flush;
  return 0;
}

int PassSubscriberTask::subscribe() {  
  PASSPortBindingProxy proxy;
  
  proxy.soap_endpoint =  PassConfigurationManager::getInstance()->getPassServerAddress().c_str();
  
  _pass__subscribe subscriptionRequest;
  subscriptionRequest.return_USCOREaddress =  PassConfigurationManager::getInstance()->getPassSubscriberAddress();
  subscriptionRequest.topic =  PassConfigurationManager::getInstance()->getPassTopic();
  subscriptionRequest.subscriber_USCOREid =  PassConfigurationManager::getInstance()->getPassSubscriberId();
  
  pass__status status;
  
  std::cout << "Subscribing..." << std::endl << std::flush;
  int result = proxy.subscribe(&subscriptionRequest, &status);
  if(result != SOAP_OK) {
    proxy.soap_print_fault(stderr);
    return 1;
  } else {
    std::cout << "Subscribed: " << status.status_USCOREcode << std::endl << std::flush;
    if(status.status_USCOREphrase) {
      std:: cout << "   (" << *(status.status_USCOREphrase) << ")" << std::endl << std::flush;
    }
  }
  
  return 0;
}

int PassSubscriberTask::unsubscribe() {
  PASSPortBindingProxy proxy;
  proxy.soap_endpoint = PassConfigurationManager::getInstance()->getPassServerAddress().c_str();
  
  _pass__unsubscribe unsubscriptionRequest;
  unsubscriptionRequest.subscriber_USCOREid =  PassConfigurationManager::getInstance()->getPassSubscriberId();
  
  pass__status status;
  
  std::cout << "Unubscribing..." << std::endl << std::flush;
  int result = proxy.unsubscribe(&unsubscriptionRequest, &status);
  if(result != SOAP_OK) {
    proxy.soap_print_fault(stderr);
    return 1;
  } else {
    std::cout << "Unubscribed: " << status.status_USCOREcode << std::endl << std::flush;
    if(status.status_USCOREphrase) {
      std:: cout << "   (" << *(status.status_USCOREphrase) << ")" << std::endl << std::flush;
    }
  }
  
  return 0;
}

PassSubscriberTask::~PassSubscriberTask() {
  
}
