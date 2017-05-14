#ifndef MESSAGE_PROCESSOR_HPP
#define MESSAGE_PROCESSOR_HPP
#include "publisher.hpp"

class message_processor
{
public:
  message_processor(publisher& p);
  void process_message(const char* msg, size_t len );

private:
  publisher& _p;
};

#endif
