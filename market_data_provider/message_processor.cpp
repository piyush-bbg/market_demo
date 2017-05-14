#include "message_processor.hpp"
#include "messages.hpp"

#include <iostream>

message_processor::message_processor(publisher& p):
  _p(p)
{  }


void message_processor::process_message(const char* msg, size_t len )
{
  int consumed_byte  = 0;
  while(len - consumed_byte)
    {
      char msg_type = *(msg + consumed_byte);
      switch(msg_type)
	{
	case 'P':
	  {
	    _p.push(reinterpret_cast<const pgu::MarketStatus*>(msg+consumed_byte));
	    consumed_byte += sizeof(pgu::MarketStatus);
	    _p.notify(1);
	    break;
	  }
	case 'M':
	  {
	    _p.push(reinterpret_cast<const pgu::Quote*>(msg+consumed_byte));
	    consumed_byte += sizeof(pgu::Quote);
	    _p.notify(2);
	    break;
	  }
	case 'T':
	  {
	    _p.push(reinterpret_cast<const pgu::Trade*>(msg+consumed_byte));
	    consumed_byte += sizeof(pgu::Trade);
	    _p.notify(4);
	    break;
	  }
	default:
	  std::cout << "Unknown message type: "<< msg_type << " ignoring packet!!"<< std::endl;
	  return;
	}
    }

}
