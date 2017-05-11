add_executable(tcp_client tcp_client.cpp)

target_link_libraries(tcp_client
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  pthread
)
