cmake_minimum_required(VERSION 3.0)

project (https-client VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++-10")

include_directories(~/boost_1_74_0)

add_definitions(-DDBOOST_ASIO_HAS_CO_AWAIT -DBOOST_ASIO_HAS_STD_COROUTINE -fcoroutines -I~/boost_1_74_0)

aux_source_directory(src SRCS)

add_executable(https-client ${SRCS})

target_link_libraries(https-client ssl crypto pthread)