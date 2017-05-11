#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <fstream>
#include <string>
#include <boost/asio.hpp>
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "messages.hpp"
const short multicast_port = 30001;
const int time_duration_per_msg = 1; // in milliseconds

class sender
{
public:
  sender(boost::asio::io_service& io_service,
	 const boost::asio::ip::address& multicast_address)
    : endpoint_(multicast_address, multicast_port),
      socket_(io_service, endpoint_.protocol()),
      timer_(io_service)
  {
    // read file messages.dat at same location
    filedesc_ = open("messages.dat", O_RDONLY);
    char buffer[128];
    read(filedesc_, buffer , 1);
    
    int len = 0;

    if (*buffer == 'P')
      {
      read(filedesc_, buffer+1 , sizeof(pgu::MarketStatus) -1);
      len = sizeof(pgu::MarketStatus);
      }

    if (*buffer == 'M')
      {
      read(filedesc_, buffer+1 , sizeof(pgu::Quote) -1);
      len = sizeof(pgu::Quote);
      }


    if (*buffer == 'T')
      {
      read(filedesc_, buffer+1 , sizeof(pgu::Trade) -1);
      len = sizeof(pgu::Trade);
      }

    if (len > 0)
      {
	socket_.async_send_to(
			      boost::asio::buffer(buffer, len), endpoint_,
			      boost::bind(&sender::handle_send_to, this,
					  boost::asio::placeholders::error));
      }
    else
      {
	timer_.expires_from_now(boost::posix_time::millisec(time_duration_per_msg));
	timer_.async_wait(
			  boost::bind(&sender::handle_timeout, this,
				      boost::asio::placeholders::error));
      }

  }

void handle_send_to(const boost::system::error_code& error)
  {
    if (!error)
      {
	timer_.expires_from_now(boost::posix_time::millisec(time_duration_per_msg));
	timer_.async_wait(
			  boost::bind(&sender::handle_timeout, this,
				      boost::asio::placeholders::error));
      }
  }

void handle_timeout(const boost::system::error_code& error)
  {
    if (!error)
      {
	char buffer[128];
	read(filedesc_, buffer , 1);
    
	int len = 0;

	if (*buffer == 'P')
	  {
	    read(filedesc_, buffer+1 , sizeof(pgu::MarketStatus) -1);
	    len = sizeof(pgu::MarketStatus);
	  }

	if (*buffer == 'M')
	  {
	    read(filedesc_, buffer+1 , sizeof(pgu::Quote) -1);
	    len = sizeof(pgu::Quote);
	  }


	if (*buffer == 'T')
	  {
	    read(filedesc_, buffer+1 , sizeof(pgu::Trade) -1);
	    len = sizeof(pgu::Trade);
	  }

	if (len > 0)
	  {
	    socket_.async_send_to(
				  boost::asio::buffer(buffer, len), endpoint_,
				  boost::bind(&sender::handle_send_to, this,
					      boost::asio::placeholders::error));
	  }
      }
  }

  ~sender()
  {
    close(filedesc_);
  }

private:
  boost::asio::ip::udp::endpoint endpoint_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::deadline_timer timer_;
  std::string message_;
  int filedesc_;
};

int main(int argc, char* argv[])
{
  try
    {
      if (argc != 2)
	{
	  std::cerr << "Usage: sender <multicast_address>\n";
	  std::cerr << "  For IPv4, try:\n";
	  std::cerr << "    sender 239.255.0.1\n";
	  std::cerr << "  For IPv6, try:\n";
	  std::cerr << "    sender ff31::8000:1234\n";
	  return 1;
	}

      boost::asio::io_service io_service;
      sender s(io_service, boost::asio::ip::address::from_string(argv[1]));
      io_service.run();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}
