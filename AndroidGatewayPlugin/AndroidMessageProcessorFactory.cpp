#include "AndroidMessageProcessorFactory.h"

AndroidMessageProcessorFactory *AndroidMessageProcessorFactory::getInstance() {
  return NULL;
}
  
AndroidMessageProcessor *AndroidMessageProcessorFactory::getMessageProcessor(std::string deviceName, std::string userName) {
  return NULL;
}

void AndroidMessageProcessorFactory::releaseMessageProcessor(AndroidMessageProcessor *processor) {
  
}

AndroidMessageProcessor *AndroidMessageProcessorFactory::getTemporaryMessageProcessor() {
  return NULL;
}
