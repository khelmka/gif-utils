cmake_minimum_required(VERSION 3.12)
project(gif-utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# argh
add_library(argh INTERFACE)
target_include_directories(argh INTERFACE
  ${CMAKE_SOURCE_DIR}/deps/argh
)

# gifdec
add_library(gifdec STATIC ${CMAKE_SOURCE_DIR}/deps/gifdec/gifdec.c)
target_compile_options(gifdec PRIVATE -O3 -w)
target_include_directories(gifdec PUBLIC
  ${CMAKE_SOURCE_DIR}/deps/gifdec/
)

# gifenc
add_library(gifenc STATIC ${CMAKE_SOURCE_DIR}/deps/gifenc/gifenc.c)
target_compile_options(gifenc PRIVATE -O3 -w)
target_include_directories(gifenc PUBLIC
  ${CMAKE_SOURCE_DIR}/deps/gifenc/
)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
  ${CMAKE_SOURCE_DIR}/deps/stb
)

add_executable(${PROJECT_NAME} main.cc)
target_compile_options(${PROJECT_NAME} PRIVATE -g -Wall)
target_link_libraries(${PROJECT_NAME} PRIVATE
  argh
  gifdec
  gifenc
  stb
)