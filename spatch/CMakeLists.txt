# Required cmake version
cmake_minimum_required (VERSION 2.8.5)

# Project language
project(C++)

# Enable C++11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if (COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif (COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else ()
  message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")

# Set project directory
set(PROJECT_F ${CMAKE_CURRENT_SOURCE_DIR}/source)

add_executable (spatch
  ${PROJECT_F}/main.cpp
  ${PROJECT_F}/AccessListController.cpp
  ${PROJECT_F}/ConfigurationController.cpp
  ${PROJECT_F}/ConfigurationReader.cpp
  ${PROJECT_F}/ServerConfiguration.cpp
  ${PROJECT_F}/Endpoint.cpp
  ${PROJECT_F}/User.cpp
  ${PROJECT_F}/ServerController.cpp
  ${PROJECT_F}/SshProxy.cpp
  ${PROJECT_F}/SshSession.cpp
  ${PROJECT_F}/SshChannel.cpp
  ${PROJECT_F}/SshClient.cpp
  ${PROJECT_F}/PseudoTerminal.cpp
)

target_link_libraries (spatch ssh)
