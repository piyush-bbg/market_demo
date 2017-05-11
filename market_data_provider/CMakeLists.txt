add_executable(market_data_provider reciever.cpp message_processor.cpp publisher.cpp tcp_sender.cpp)

target_link_libraries(market_data_provider
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  pthread
)

