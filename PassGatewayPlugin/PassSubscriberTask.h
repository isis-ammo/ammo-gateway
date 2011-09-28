#ifndef PASS_SUBSCRIBER_TASK_H
#define PASS_SUBSCRIBER_TASK_H

#include "ace/Task.h"

class PassConfigurationManager;

class PassSubscriberTask : public ACE_Task <ACE_MT_SYNCH>
{
public:
  PassSubscriberTask (void);
  virtual ~PassSubscriberTask (void);
  
  virtual int open (void *args = 0);
  virtual int close (unsigned long flags = 0);
  virtual int svc (void);

private:
  int subscribe (void);
  int unsubscribe (void);
  
private:
  PassConfigurationManager *cfg_mgr_;
  bool closed_;
};

#endif // PASS_SUBSCRIBER_TASK_H
