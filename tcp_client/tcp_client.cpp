#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>

#include "messages.hpp"


using boost::asio::ip::tcp;

class client
{
public:
  client(boost::asio::io_service& io_service, char* sub_type)
    : stopped_(false),
      socket_(io_service)
     , _remaining_data(0)
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
	std::cout << "Not able to convert subscription to integer. Assigning value 7 (Market_status + Quote + Trade)" << std::endl;
	sub_type_ = 7;
      }
  }

  void start(tcp::resolver::iterator endpoint_iter)
  {
    start_connect(endpoint_iter);
  }

  void stop()
  {
    stopped_ = true;
    socket_.close();
  }
private:
  void start_connect(tcp::resolver::iterator endpoint_iter)
  {
    if (endpoint_iter != tcp::resolver::iterator())
      {
	std::cout << "Trying " << endpoint_iter->endpoint() << "...\n";

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
	start_connect(++endpoint_iter);
      }

    else if (ec)
      {
	std::cout << "Connect error: " << ec.message() << "\n";

	socket_.close();

	// Try the next available endpoint.
	start_connect(++endpoint_iter);
      }

    // Otherwise we have successfully established a connection.
    else
      {
	std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
	std::cout << "Message_type information: P - Market status, M - Quote, T - Trade\n";

	start_write();
	start_read();

      }
  }

  void start_read()
  {
    boost::asio::async_read(socket_,
			    boost::asio::buffer(data_+ _remaining_data, max_length - _remaining_data),
			    boost::asio::transfer_at_least(1),
			    boost::bind(&client::handle_read, this, _1, _2));
  }

  void handle_read(const boost::system::error_code& ec, size_t byte_length)
  {
    tot_length += byte_length;
    if (stopped_)
      return;

    if (!ec)
      {
	size_t read_len = byte_length + _remaining_data;
	_remaining_data = print_msg(data_, read_len);
	if (_remaining_data)
	  memmove(data_, data_+ read_len - _remaining_data, _remaining_data);

	start_read();
      }
    else
      {
	std::cout << "Error on receive: " << ec.message() << "\n";

	stop();
      }
  }

int print_msg(char* msg, size_t byte_length)
  {
    bool f_exit = false;
    int consumed_byte = 0;
    int byte_left = byte_length;
    while( byte_left )
      {
	char msg_type = *( msg + consumed_byte);
	switch(msg_type)
	  {
	  case 'P':
	    {
	      if (byte_left < sizeof(pgu::MarketStatus))
		f_exit =true;
	      else
		{
	      const pgu::MarketStatus* ms =
		reinterpret_cast<const pgu::MarketStatus*>(msg + consumed_byte);
	      consumed_byte += sizeof(pgu::MarketStatus);
	      byte_left -= sizeof(pgu::MarketStatus);
	      std::cout << *ms << std::endl;
		}
	      break;
	    }
	  case 'M':
	    {
	      if (byte_left < sizeof(pgu::Quote))
		f_exit = true;
	      else
		{
	      const pgu::Quote* q = reinterpret_cast<const pgu::Quote*>(msg + consumed_byte);
	      consumed_byte += sizeof(pgu::Quote);
	      byte_left -= sizeof(pgu::Quote);
	      std::cout << *q << std::endl;
		}
	      break;
	    }
	  case 'T':
	    {
	      if (byte_left < sizeof(pgu::Trade))
		f_exit =true;
	      else
		{
	      const pgu::Trade* t = reinterpret_cast<const pgu::Trade*>(msg + consumed_byte);
	      consumed_byte += sizeof(pgu::Trade);
	      byte_left -= sizeof(pgu::Trade);
	      std::cout << *t << std::endl;
		}
	      break;
	    }
	  default:
	    std::cout << "Unknown message type: "<< msg_type << " ignoring packet!!"<< std::endl;
	    return 0;
	  }
	if (f_exit)
	  break;
      }
    //   std:: cout << "byte_length: " << byte_length << " consumed_byte: " << consumed_byte << " byte_left: "<< byte_left<<std::endl;
    return byte_left;
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
	//std::cout << "Error on heartbeat: " << ec.message() << "\n";
	stop();
      }
  }


private:
  bool stopped_;
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
  short sub_type_;
  size_t _remaining_data;
  size_t tot_length;
};

int main(int argc, char* argv[])
{
  try
    {
      if (argc != 4)
	{
	  std::cerr << "Usage: tcp_client <host> <port> <subscription_type>\n";
	  std::cerr << "<subscription_type> 1 Market status, 2 Quote, 4 Trade\n";
	  std::cerr << "<subscription_type> combination 3 (Market status +  Quote)\n";
	  std::cerr << "<subscription_type> combination 5 (Market status +  Trade)\n";
	  std::cerr << "<subscription_type> combination 6 (Quote +  Trade)\n";
	  std::cerr << "<subscription_type> combination 7 All (Quote +  Trade + Market Status)\n";

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
