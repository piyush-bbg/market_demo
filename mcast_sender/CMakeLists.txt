add_executable(mcast_sender sender.cpp)

target_link_libraries(mcast_sender
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  pthread
)

