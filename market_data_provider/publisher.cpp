#include <iostream>
#include <boost/shared_ptr.hpp>

#include "publisher.hpp"
#include "tcp_sender.hpp"

publisher::publisher()
{
  // try
  //   {
  //     std::cout << "creating server on port 40001" << std::endl;     
  //   }
  //   catch (std::exception& e)
  //   {
  //     std::cerr << "Exception: server cannot be created" << e.what() << "\n";
  //     throw;
  //   }
}

void publisher::run()
{
  std::cout << "publisher started" << std::endl;
  s = new server(_io_service, tcp_port, this);
  _io_service.run();
}

publisher::~publisher()
{
  // close connection object
}


void publisher::subscribe(subscriber* s, char msg_type) 
  {
    	switch(msg_type)
	  {
	  case 'P': 
	    {
	      _ms_subscribers.push_back(s); 
	    break;
	    }
	  case 'M':
	    {
	      _quote_subscribers.push_back(s); 
	      break;
	    }
	  case 'T':
	    {
	      _trade_subscribers.push_back(s); 
	      break;
	    }
	  default:
	    std::cout << "Unknown message type subscription" << std::endl;
	    
	  }    
  }

void publisher::unsubscribe(subscriber* s, char msg_type) 
  {
  	switch(msg_type)
	  {
	  case 'P': 
	    {
	      _ms_subscribers.remove(s); ; 
	    break;
	    }
	  case 'M':
	    {
	      _quote_subscribers.remove(s); 
	      break;
	    }
	  case 'T':
	    {
	      _trade_subscribers.remove(s); 
	      break;
	    }
	  default:
	    std::cout << "Unknown message type subscription" << std::endl;
	    
	  }
  }

void publisher::notify(const char* msg, size_t len)
  {
    char msg_type = *msg;
    std::list<subscriber*>::iterator p;
    	switch(msg_type)
	  {
	  case 'P': 
	    {
	      std::cout << "Notify for Market Status called" << std::endl;
	      for( p = _ms_subscribers.begin(); p != _ms_subscribers.end(); p++)
		(*p)->update(msg, len);
	      break;
	    }
	  case 'M':
	    {
	      std::cout << "Notify for Quote called" << std::endl;
	      for( p = _quote_subscribers.begin(); p != _quote_subscribers.end(); p++)
		(*p)->update(msg, len);
	      break;
	    }
	  case 'T':
	    {
	      std::cout << "Notify for Trade called" << std::endl;
	      for( p = _trade_subscribers.begin(); p != _trade_subscribers.end(); p++)
		(*p)->update(msg, len);
	      break;
	    }
	  default:
	    std::cout << "Should never reach here!!!" << std::endl;
	  }    
  }

