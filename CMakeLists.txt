cmake_minimum_required(VERSION 3.15)
project(cache-sim)

set(CMAKE_CXX_STANDARD 14)

add_executable(cache-sim src/main.cpp src/cache.cpp src/cache.hpp src/memory.cpp src/memory.hpp src/address_translator.cpp src/address_translator.hpp src/errors.cpp src/errors.hpp src/driver.cpp src/driver.hpp)