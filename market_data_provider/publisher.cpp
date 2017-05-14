#include <iostream>
#include <boost/shared_ptr.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

#include "publisher.hpp"
#include "tcp_sender.hpp"

const int default_queue_size = 1024;

publisher::publisher()
  :
  _ms_queue(default_queue_size),
  _q_queue(default_queue_size),
  _t_queue(default_queue_size)
{
}

void publisher::start()
{
  std::cout << "Publisher started" << std::endl;
  _s = new server(_adw.get_io_service(), tcp_port, this);
  _adw.start();
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
    // boost::mutex::scoped_lock lock(_sub_mutex);
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

void publisher::push(const pgu::MarketStatus* msg)
  {
    _ms_queue.push(*msg);
  }

void publisher::push(const pgu::Quote* msg)
  {
    _q_queue.push(*msg);
  }

void publisher::push(const pgu::Trade* msg)
  {
    _t_queue.push(*msg);
  }


void publisher::notify(int data_type)
  {
    int temp = _data_availiable.load();
    temp = temp | data_type;
    _data_availiable.store(temp);

    _cond.notify_one();
  }

void publisher::observer()
{
  // This should be called by main thread
  while(1)
    {
      boost::unique_lock<boost::mutex> lock(_mut);
      int flag = _data_availiable.load();
      while(flag == 0)
	{
	  _cond.wait(lock);
	  //	  std::cout << "publisher::observer " << std::endl;
	  flag = _data_availiable.load();
	}
      process_data();
    }
}

void publisher::process_data()
{

  int msg_type =  _data_availiable.load();

  if (msg_type & 0x0001)
    {
      msg_type &= 0xfffe;
      _data_availiable.store(msg_type);
      update_market_status();
    }
  else  if (msg_type & 0x0002)
    {
      msg_type &= 0xfffd;
      _data_availiable.store(msg_type);
      update_quote();
    }
  else  if (msg_type & 0x0004)
    {
      msg_type &= 0xfffb;
      _data_availiable.store(msg_type);
      update_trade();
    }
}

void publisher::update_market_status()
{
  pgu::MarketStatus msg;
  while(_ms_queue.pop(msg))
    {
      std::list<subscriber*>::iterator p;
      //      std::cout << "MarketStatus: " << msg << std::endl;
      for( p = _ms_subscribers.begin(); p != _ms_subscribers.end(); p++)
	{
	  (*p)->update(reinterpret_cast<const char*>(&msg), sizeof(msg));
	}
    }
}

void publisher::update_quote()
{
  pgu::Quote msg;
  while(_q_queue.pop(msg))
    {
	std::list<subscriber*>::iterator p;
	//	std::cout << "Quote: " << msg << std::endl;
	for( p = _quote_subscribers.begin(); p != _quote_subscribers.end(); p++)
	{
	  (*p)->update(reinterpret_cast<const char*>(&msg), sizeof(msg));
	}
    }
}

void publisher::update_trade()
{
  pgu::Trade msg;
  while(_t_queue.pop(msg))
    {
	std::list<subscriber*>::iterator p;
	//	std::cout << "Trade: " << msg << std::endl;
	for( p = _trade_subscribers.begin(); p != _trade_subscribers.end(); p++)
	{
	  (*p)->update(reinterpret_cast<const char*>(&msg), sizeof(msg));
	}
    }
}
