#include "../include/messages.hpp"
#include "message_processor.hpp"
#include <iostream>

void message_processor::process_message(const char* msg, size_t len )
{
  char msg_type = *msg;    
  std:: cout << "message_processor::process_message msg_type: " << msg_type << std::endl;
  switch(msg_type)
    {
    case 'P': 
      {
	_p.notify(msg, len);
	break;
      }
    case 'M':
      {
	_p.notify(msg, len);
	break;
      }
    case 'T':
      {
	_p.notify(msg, len);
	break;
      }
    default:
      std::cout << "Unknown message type: "<< msg_type << std::endl;      
    }
}

