#ifndef ANDROID_MESSAGE_PROCESSOR_FACTORY_H
#define ANDROID_MESSAGE_PROCESSOR_FACTORY_H

#include <string>
#include <map>

class AndroidMessageProcessor;

class AndroidMessageProcessorFactory {
public:
  static AndroidMessageProcessorFactory *getInstance();
  
  AndroidMessageProcessor *getMessageProcessor(std::string deviceName, std::string userName);
  void releaseMessageProcessor(AndroidMessageProcessor *processor);
  
  AndroidMessageProcessor *getTemporaryMessageProcessor();
  
private:
  static AndroidMessageProcessorFactory *sharedInstance;
  
  std::map<std::string, AndroidMessageProcessor *> messageProcessors;
};

#endif //ANDROID_MESSAGE_PROCESSOR_FACTORY_H
