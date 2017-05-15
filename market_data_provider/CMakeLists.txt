if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(market_data_provider reciever.cpp message_processor.cpp publisher.cpp tcp_sender.cpp)
  target_link_libraries(market_data_provider
    ${Boost_LIBRARIES}
    pthread
    )

endif()

