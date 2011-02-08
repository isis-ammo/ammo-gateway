#ifndef ANDROID_MESSAGE_PROCESSOR_FACTORY_H
#define ANDROID_MESSAGE_PROCESSOR_FACTORY_H

#include <string>

class AndroidMessageProcessor;

class AndroidMessageProcessorFactory {
public:
  static AndroidMessageProcessorFactory *getInstance();
  
  AndroidMessageProcessor *getMessageProcessor(std::string deviceName, std::string userName);
  void releaseMessageProcessor(AndroidMessageProcessor *processor);
  
  AndroidMessageProcessor *getTemporaryMessageProcessor();
  
private:
};

#endif //ANDROID_MESSAGE_PROCESSOR_FACTORY_H
