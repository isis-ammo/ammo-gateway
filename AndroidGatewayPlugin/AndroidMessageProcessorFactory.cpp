#include "AndroidMessageProcessorFactory.h"
#include "AndroidMessageProcessor.h"

using namespace std;

AndroidMessageProcessorFactory *AndroidMessageProcessorFactory::sharedInstance = NULL;

AndroidMessageProcessorFactory *AndroidMessageProcessorFactory::getInstance() {
  if(sharedInstance == NULL) {
    sharedInstance = new AndroidMessageProcessorFactory();
  }
  return sharedInstance;
}
  
AndroidMessageProcessor *AndroidMessageProcessorFactory::getMessageProcessor(std::string deviceName, std::string userName) {
  map<string, AndroidMessageProcessor *>::iterator processor = messageProcessors.find(deviceName + "/" + userName);
  
  if(processor == messageProcessors.end()) {
    AndroidMessageProcessor *newProcessor = new AndroidMessageProcessor(NULL);
    messageProcessors[deviceName + "/" + userName] = newProcessor;
    return newProcessor;
  }
  
  return processor->second;
}

void AndroidMessageProcessorFactory::releaseMessageProcessor(AndroidMessageProcessor *processor) {
  
}

AndroidMessageProcessor *AndroidMessageProcessorFactory::getTemporaryMessageProcessor() {
  return NULL;
}
