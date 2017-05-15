if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(tcp_client tcp_client.cpp)

  target_link_libraries(tcp_client
    ${Boost_LIBRARIES}
    pthread
    )
endif()

