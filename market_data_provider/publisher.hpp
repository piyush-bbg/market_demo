#ifndef PUBLISHER_HPP
#define PUBLISHER_HPP
#include "../include/handler.hpp"
#include "async_data_writer.hpp"
#include "tcp_sender.hpp"

#include <list>
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/condition.hpp>

class subscriber;

class publisher : public handler
{
public:
  publisher();
  ~publisher();
  void subscribe(subscriber* s, char msg_type) ;
  void unsubscribe(subscriber* s, char msg_type) ;

  void notify(int);
  void start();

  void push(const pgu::MarketStatus* msg);
  void push(const pgu::Quote* msg);
  void push(const pgu::Trade* msg);

  void observer();

private:
  void process_data();
  void update_market_status();
  void update_quote();
  void update_trade();

  std::list<subscriber*> _ms_subscribers;
  std::list<subscriber*> _quote_subscribers;
  std::list<subscriber*> _trade_subscribers;

  async_data_writer _adw;
  server* _s;
  boost::mutex _mut;
  boost::condition_variable _cond;

  boost::lockfree::queue<pgu::MarketStatus> _ms_queue;
  boost::lockfree::queue<pgu::Quote> _q_queue;
  boost::lockfree::queue<pgu::Trade> _t_queue;

public:
  boost::atomic<int> _data_availiable;
};
#endif
