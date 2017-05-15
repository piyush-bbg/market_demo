if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(mcast_sender sender.cpp)

  target_link_libraries(mcast_sender
    ${Boost_LIBRARIES}
    pthread
    )
endif()
