#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

#include "../include/messages.hpp"


using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

class client
{
public:
  client(boost::asio::io_service& io_service, char* sub_type)
    : stopped_(false),
      socket_(io_service)
      //      deadline_(io_service),
      //      heartbeat_timer_(io_service)
  {
    try
      {
	sub_type_ = boost::lexical_cast<int>(sub_type);
	if (sub_type_ < 1 && sub_type_ > 7)
	  {
	    sub_type_ = 7;
	    std::cout << "Assigning default";
	  }
	std::cout << "Subscription type: " << sub_type_ << std::endl;
      }
    catch(const boost::bad_lexical_cast &)
      {
	std::cout << "Not able to convert subscription to integer. Assigning value 7 (Marke_status + Quote + Trade)" << std::endl;
	sub_type_ = 7;
      }
  }

  void start(tcp::resolver::iterator endpoint_iter)
  {

    start_connect(endpoint_iter);
    // deadline_.async_wait(boost::bind(&client::check_deadline, this));
  }

  void stop()
  {
    stopped_ = true;
    socket_.close();
    // deadline_.cancel();
    //    heartbeat_timer_.cancel();
  }

private:
  void start_connect(tcp::resolver::iterator endpoint_iter)
  {
    if (endpoint_iter != tcp::resolver::iterator())
      {
	std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

	// Set a deadline for the connect operation.
	//deadline_.expires_from_now(boost::posix_time::seconds(60));

	// Start the asynchronous connect operation.
	socket_.async_connect(endpoint_iter->endpoint(),
			      boost::bind(&client::handle_connect,
					  this, _1, endpoint_iter));
      }
    else
      {
	// There are no more endpoints to try. Shut down the client.
	stop();
      }
  }

  void handle_connect(const boost::system::error_code& ec,
		      tcp::resolver::iterator endpoint_iter)
  {
    if (stopped_)
      return;

    if (!socket_.is_open())
      {
	std::cout << "Connect timed out\n";

	// Try the next available endpoint.
	start_connect(++endpoint_iter);
      }

    // Check if the connect operation failed before the deadline expired.
    else if (ec)
      {
	std::cout << "Connect error: " << ec.message() << "\n";

	// We need to close the socket used in the previous connection attempt
	// before starting a new one.
	socket_.close();

	// Try the next available endpoint.
	start_connect(++endpoint_iter);
      }

    // Otherwise we have successfully established a connection.
    else
      {
	std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

	start_write();

	start_read();

      }
  }

  void start_read()
  {

    // Set a deadline for the read operation.
    //  deadline_.expires_from_now(boost::posix_time::seconds(30));

    // Start an asynchronous operation to read a newline-delimited message.
    boost::asio::async_read(socket_, boost::asio::buffer(data), boost::asio::transfer_at_least(1),
			    boost::bind(&client::handle_read, this, _1));
  }

  void handle_read(const boost::system::error_code& ec)
  {
    if (stopped_)
      return;

    if (!ec)
      {
	print_msg(data);
	start_read();
      }
    else
      {
	std::cout << "Error on receive: " << ec.message() << "\n";

	stop();
      }
  }

  void print_msg(char* msg)
  {
    char msg_type = *msg;    
    std:: cout << "Message_type:**********";
	switch(msg_type)
	  {
	  case 'P': 
	    {
	    const pgu::MarketStatus* ms = 
	      reinterpret_cast<const pgu::MarketStatus*>(msg);
	    std::cout << *ms << std::endl;
	    break;
	    }
	  case 'M':
	    {
	      const pgu::Quote* q =
	       reinterpret_cast<const pgu::Quote*>(msg);
	    std::cout << *q << std::endl;
	    break;
	    }
	  case 'T':
	    {
	    const pgu::Trade* t =
	      reinterpret_cast<const pgu::Trade*>(msg);
	    std::cout << *t << std::endl;
	    break;
	    }
	  default:
	    std::cout << "Unknown message type" << std::endl; 
	  }
  }


  void start_write()
  {
    if (stopped_)
      return;

    pgu::Logon msg;
    msg.type = 'L';
    msg.id = 10001;
    msg.subscription = sub_type_;
    
    // start an asynchronous operation to send a heartbeat message.
    boost::asio::async_write(socket_, boost::asio::buffer(&msg, sizeof(msg)),
			     boost::bind(&client::handle_write, this, _1));
  }

  void handle_write(const boost::system::error_code& ec)
  {
    if (stopped_)
      return;

    if (ec)
      {
	std::cout << "Error on heartbeat: " << ec.message() << "\n";

	stop();
      }
  }

  // void check_deadline()
  // {
  //   if (stopped_)
  //     return;

  //   // Check whether the deadline has passed. We compare the deadline against
  //   // the current time since a new asynchronous operation may have moved the
  //   // deadline before this actor had a chance to run.
  //   if (deadline_.expires_at() <= deadline_timer::traits_type::now())
  //     {
  // 	// The deadline has passed. The socket is closed so that any outstanding
  // 	// asynchronous operations are cancelled.
  // 	socket_.close();

  // 	// There is no longer an active deadline. The expiry is set to positive
  // 	// infinity so that the actor takes no action until a new deadline is set.
  // 	deadline_.expires_at(boost::posix_time::pos_infin);
  //     }

  //   // Put the actor back to sleep.
  //   deadline_.async_wait(boost::bind(&client::check_deadline, this));
  // }

private:
  bool stopped_;
  tcp::socket socket_;
  //  deadline_timer deadline_;
  //  deadline_timer heartbeat_timer_;
  char data[1024];
  short sub_type_;
};

int main(int argc, char* argv[])
{
  try
    {
      if (argc != 4)
	{
	  std::cerr << "Usage: client <host> <port> <subscription_type>\n";
	  std::cerr << "<subscription_type> 1 Market status, 2 Quote, 4 Trade\n";
	  std::cerr << "<subscription_type> combination 3 Market status +  Quote\n";
	  std::cerr << "<subscription_type> combination 5 Market status +  Trade\n";
	  std::cerr << "<subscription_type> combination 6 Quote +  Trade\n";
	  std::cerr << "<subscription_type> combination 7 All  Quote +  Trade + Market Status\n";

	  return 1;
	}

      boost::asio::io_service io_service;
      tcp::resolver r(io_service);
      client c(io_service, argv[3]);

      c.start(r.resolve(tcp::resolver::query(argv[1], argv[2])));

      io_service.run();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}
