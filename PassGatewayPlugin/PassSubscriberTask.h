#ifndef PASS_SUBSCRIBER_TASK_H
#define PASS_SUBSCRIBER_TASK_H

#include "ace/Task.h"

class PassSubscriberTask : public ACE_Task <ACE_MT_SYNCH> {
public:
  PassSubscriberTask();
  virtual ~PassSubscriberTask();
  
  virtual int open(void *args);
  virtual int close(unsigned long flags);
  
  virtual int svc();
  
private:
  bool closed;
  
  int subscribe();
  int unsubscribe();
};

#endif // PASS_SUBSCRIBER_TASK_H
