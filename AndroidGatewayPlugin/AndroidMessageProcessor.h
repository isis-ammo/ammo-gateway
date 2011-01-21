#ifndef ANDROID_MESSAGE_PROCESSOR_H
#define ANDROID_MESSAGE_PROCESSOR_H

#include "ace/Task.h"
#include "protocol/AmmoMessages.pb.h"

class AndroidServiceHandler;

class AndroidMessageProcessor : public ACE_Task <ACE_MT_SYNCH> {
public:
  AndroidMessageProcessor(AndroidServiceHandler *serviceHandler);
  virtual ~AndroidMessageProcessor();
  
  virtual int open(void *args);
  virtual int close(unsigned long flags);
  
  virtual int svc();
  
  void signalNewMessageAvailable();
  
private:
  bool closed;
  ACE_Thread_Mutex closeMutex;
  ACE_Thread_Mutex newMessageMutex;
  ACE_Condition_Thread_Mutex newMessageAvailable;
  
  AndroidServiceHandler *commsHandler;
  
  bool isClosed();
  void processMessage(ammo::protocol::MessageWrapper &msg);
};

#endif
