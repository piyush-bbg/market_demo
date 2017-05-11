#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP
#include "../include/handler.hpp"
#include "tcp_sender.hpp"
#include <list>
#include <boost/asio.hpp>

class subscriber;

class publisher : public handler
{
public:
  publisher();
  ~publisher();
  void subscribe(subscriber* s, char msg_type) ;
  void unsubscribe(subscriber* s, char msg_type) ;
  void notify(const char* msg, size_t len);
  void run();
private:
  
  std::list<subscriber*> _ms_subscribers;
  std::list<subscriber*> _quote_subscribers;
  std::list<subscriber*> _trade_subscribers;
  boost::asio::io_service _io_service;
  server* s;
};


#endif
