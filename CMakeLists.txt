cmake_minimum_required (VERSION 2.6)
project (MARKET_DATA)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

include_directories ("${PROJECT_SOURCE_DIR}/include")
add_subdirectory(market_data_provider)
add_subdirectory(mcast_sender)
add_subdirectory(tcp_client)