#ifndef ASYNC_DATA_WRITER_HPP
#define ASYNC_DATA_WRITER_HPP

#include <boost/noncopyable.hpp>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include <string>
#include <signal.h>


class async_data_writer : boost::noncopyable
{
public:
    async_data_writer( )
      : _running(false)
    {}

    ~async_data_writer()
    {
        stop();

        if (_thread)
            _thread->join();
    }

    boost::asio::io_service & get_io_service()
    {
        return _io_service;
    }

    void start()
    {
        if (_running)
            throw std::logic_error("Thread was already started.");

        if (_thread)
            _thread->join();

        _work.reset( new boost::asio::io_service::work(_io_service) );

	_thread.reset( new boost::thread(&async_data_writer::run,
                                         this));


        size_t timeout = 1000; // 1 sec timeout
        while (!_running && timeout-- > 0)
            usleep(1000); // 1 millisecond
    }

    void stop()
    {
        if (_running)
        {
            _running = false;
            _work.reset();
       }
    }

    void consume()
    {
      
      
      run();
    }

private:
    void run()
    {
        _running = true;

       while(1)
        {
            try
            {
                _io_service.run();
                break;
            }
            catch (const boost::system::system_error& e)
            {
	      std::cout<<"async_data_writer::run " << e.what()<< std::endl;
            }
        }
    }

    boost::asio::io_service _io_service;
    std::auto_ptr<boost::asio::io_service::work> _work;

    boost::shared_ptr<boost::thread> _thread;
    volatile bool _running;
};

#endif
