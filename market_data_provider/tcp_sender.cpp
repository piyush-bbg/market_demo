#include <cstdlib>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "tcp_sender.hpp"

using boost::asio::ip::tcp;

session::session(boost::asio::io_service& io_service,
	  handler* h)
    : socket_(io_service),
      handler_( h ),
      sub_type_(0)
  {
  }

  tcp::socket& session::socket()
  {
    return socket_;
  }

  void session::start()
  {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
			    boost::bind(&session::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
  }

  session::~session()
  {
    switch(sub_type_)
      {
      case 1:
  	handler_->unsubscribe(this, 'P');
  	break;
      case 2:
  	handler_->unsubscribe(this, 'M');
  	break;
      case 3:
  	handler_->unsubscribe(this, 'P');
  	handler_->unsubscribe(this, 'M');
  	break;
      case 4:
  	handler_->unsubscribe(this, 'T');
  	break;
      case 5:
  	handler_->unsubscribe(this, 'P');
  	handler_->unsubscribe(this, 'T');
  	break;
      case 6:
  	handler_->unsubscribe(this, 'M');
  	handler_->unsubscribe(this, 'T');
  	break;
      case 7:
  	handler_->unsubscribe(this, 'P');
  	handler_->unsubscribe(this, 'M');
  	handler_->unsubscribe(this, 'T');
  	break;
      default:
  	std::cout << "No unsubscription done" << std::endl;
      }

  }
    

  void session::handle_read(const boost::system::error_code& error,
		   size_t bytes_transferred)
  {
    if (!error)
      {
	char msg_type = *data_;
	std::cout << "session::handle_read:msg_type " << msg_type << std::endl;

	switch(msg_type)
	  {
	  case 'L':
	    {
	      pgu::Logon* msg = reinterpret_cast<pgu::Logon*>(data_);
	      on_logon_msg(msg);
	    }
	    break;
	  case 'X':
	    {
	      pgu::Logout* msg = reinterpret_cast<pgu::Logout*>(data_);
	      on_logout_msg(msg);
	    }
	    break;
	  case 'H':
	    {
	      pgu::Heartbeat* msg = reinterpret_cast<pgu::Heartbeat*>(data_);
	      on_heartbeat_msg(msg);
	    }
	    break;
	  default:
	    std::cout << "Unknown message type send" << std::endl;
	  }
      }
  }


  void session::handle_write(const boost::system::error_code& error)
  {
    if (!error)
      {
	// socket_.async_read_some(boost::asio::buffer(data_, max_length),
	// 			boost::bind(&session::handle_read, this,
	// 				    boost::asio::placeholders::error,
	// 				    boost::asio::placeholders::bytes_transferred));
      }

  }

  void session::update(const char* msg, size_t len)
  {
    	boost::asio::async_write(socket_,
				 boost::asio::buffer(msg, len),
				 boost::bind(&session::handle_write, this,
					     boost::asio::placeholders::error));

  }

  void session::on_logon_msg(pgu::Logon* msg)
  {
    
    std::cout << "session::on_logon_msg sub_type:" << msg->subscription << std::endl;
    switch(msg->subscription)
      {
      case 1:
  	handler_->subscribe(this, 'P');
  	break;
      case 2:
  	handler_->subscribe(this, 'M');
  	break;
      case 3:
  	handler_->subscribe(this, 'P');
  	handler_->subscribe(this, 'M');
  	break;
      case 4:
  	handler_->subscribe(this, 'T');
  	break;
      case 5:
  	handler_->subscribe(this, 'P');
  	handler_->subscribe(this, 'T');
  	break;
      case 6:
  	handler_->subscribe(this, 'M');
  	handler_->subscribe(this, 'T');
  	break;
      case 7:
  	handler_->subscribe(this, 'P');
  	handler_->subscribe(this, 'M');
  	handler_->subscribe(this, 'T');
  	break;
      default:
  	std::cout << "No subscription done: Unknown subscription type" << std::endl;

      }

  }

void session::on_logout_msg(pgu::Logout* msg)
{
  // send logout message and close the connection
  
  delete this;
}


void session::on_heartbeat_msg(pgu::Heartbeat* msg)
{
  // reset the timer
}


// server class       

server::server(boost::asio::io_service& io_service, short port, handler* h)
    : io_service_(io_service),
      acceptor_(io_service, tcp::endpoint(tcp::v4(), port)),
      handler_(h)
  {
    start_accept();
  }

void server::start_accept()
  {
    session* new_session = new session(io_service_, handler_);
    acceptor_.async_accept(new_session->socket(),
			   boost::bind(&server::handle_accept, this, new_session,
				       boost::asio::placeholders::error));
  }

void server::handle_accept(session* new_session,
			   const boost::system::error_code& error)
{
  if (!error)
    {
      new_session->start();
    }
  else
    {
      delete new_session;
    }

  start_accept();
}

