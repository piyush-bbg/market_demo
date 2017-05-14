#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "async_data_reader.hpp"
#include "message_processor.hpp"

const short multicast_port = 30001;

class receiver
{
public:
  receiver(async_data_reader& adr,
	   const boost::asio::ip::address& listen_address,
	   const boost::asio::ip::address& multicast_address)
    : socket_(adr.get_io_service())
    , processor_(pub_)
  {
    // Create the socket so that multiple may be bound to the same address.
    boost::asio::ip::udp::endpoint listen_endpoint(listen_address, multicast_port);
    socket_.open(listen_endpoint.protocol());
    socket_.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    socket_.bind(listen_endpoint);

    // Join the multicast group.
    socket_.set_option(boost::asio::ip::multicast::join_group(multicast_address));

    socket_.async_receive_from(
			       boost::asio::buffer(data_, max_length), sender_endpoint_,
			       boost::bind(&receiver::handle_receive_from, this,
					   boost::asio::placeholders::error,
					   boost::asio::placeholders::bytes_transferred));


    adr.start();
    std::cout << "Reciever started" << std::endl;
    pub_.start();
    pub_.observer();
  }

void handle_receive_from(const boost::system::error_code& error,
			 size_t bytes_recvd)
{
  if (!error)
    {
      processor_.process_message(data_, bytes_recvd);

      // std::cout.write(data_, bytes_recvd);
      // std::cout << std::endl;

      socket_.async_receive_from(
				 boost::asio::buffer(data_, max_length), sender_endpoint_,
				 boost::bind(&receiver::handle_receive_from, this,
					     boost::asio::placeholders::error,
					     boost::asio::placeholders::bytes_transferred));
    }
}

  ~receiver()
  {
  }

private:
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint sender_endpoint_;
  enum { max_length = 1024 };
  char data_[max_length];
  publisher pub_;
  message_processor processor_;
};

int main(int argc, char* argv[])
{
  try
    {
      if (argc != 3)
	{
	  std::cerr << "Usage: market_data_provider <listen_address> <multicast_address>\n";
	  std::cerr << "  For IPv4, try:\n";
	  std::cerr << "    market_data_provider 0.0.0.0 239.255.0.1\n";
	  std::cerr << "  For IPv6, try:\n";
	  std::cerr << "    market_data_provider 0::0 ff31::8000:1234\n";
	  return 1;
	}

      async_data_reader adr;
      receiver r( adr,
		  boost::asio::ip::address::from_string(argv[1]),
		  boost::asio::ip::address::from_string(argv[2]));
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception: " << e.what() << "\n";
    }

  return 0;
}
