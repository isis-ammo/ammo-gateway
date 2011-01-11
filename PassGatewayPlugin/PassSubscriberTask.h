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
