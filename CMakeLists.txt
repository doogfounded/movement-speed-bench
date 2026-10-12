cmake_minimum_required(VERSION 3.20)
project(movement_speed_bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(movement_model INTERFACE)
target_include_directories(movement_model INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

function(configure_target target)
  target_link_libraries(${target} PRIVATE movement_model)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endfunction()

add_executable(movement_bench src/main.cpp)
configure_target(movement_bench)

add_executable(movement_tests tests/movement_tests.cpp)
configure_target(movement_tests)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME movement_tests COMMAND movement_tests)
endif()
