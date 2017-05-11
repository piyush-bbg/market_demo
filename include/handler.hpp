#ifndef handler_hpp
#define handler_hpp

class subscriber;

class handler
{
public:
  virtual void subscribe(subscriber* s, char msg_type) = 0;
  virtual void unsubscribe(subscriber* s, char msg_type) = 0;
};

#endif
