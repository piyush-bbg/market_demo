#ifndef TCP_SENDER_HPP
#define TCP_SENDER_HPP
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../include/messages.hpp"
#include "../include/subscriber.hpp"
#include "../include/handler.hpp"

using boost::asio::ip::tcp;
const short tcp_port = 40001;

class session : public subscriber
{
public:
  session(boost::asio::io_service& io_service,
	  handler* h);

  tcp::socket& socket();

  void start();

  ~session();
    
private:
  void handle_read(const boost::system::error_code& error,
		   size_t bytes_transferred);
  void handle_write(const boost::system::error_code& error);

  void update (const char* msg, size_t len);

  void on_logon_msg(pgu::Logon* msg);


  void on_logout_msg(pgu::Logout* msg);


  void on_heartbeat_msg(pgu::Heartbeat* msg);

       
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  handler*  handler_;
  short sub_type_;
};

class server 
{
public:
  server(boost::asio::io_service& io_service, short port, handler* h);

private:
  void start_accept();

  void handle_accept(session* new_session,
		     const boost::system::error_code& error);

  boost::asio::io_service& io_service_;
  tcp::acceptor acceptor_;
  handler*  handler_;
};

#endif
