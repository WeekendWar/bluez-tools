cmake_minimum_required(VERSION 3.5)

project(bluez_tools LANGUAGES C)
set(PROJECT_VERSION 1.1.0)

# Default to C++14
# if(NOT CMAKE_CXX_STANDARD)
#   set(CMAKE_CXX_STANDARD 14)
# endif()

# if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#   add_compile_options(-Wall -Wextra -Wpedantic)
# endif()

###############################################################################
## Configure file locations                                                  ##
###############################################################################
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(glib REQUIRED)

set(SRC
  src/bt-agent.c
  src/lib/agent-helper.c
  src/lib/dbus-common.c
  # src/lib/helpers.c
  # src/lib/manager.c
  # src/lib/properties.c
  # src/lib/sdp.c
  # src/lib/bluez/adapter.c
  # src/lib/bluez/adapter.h
  src/lib/bluez/agent_manager.c
  src/lib/bluez/agent_manager.h
)

message(STATUS "Building bt-agent...")
add_executable(bt-agent
               ${SRC})

target_include_directories(bt-agent PUBLIC
                           src/lib/
                           src/lib/bluez/)

target_link_libraries(bt-agent
                      pthread
                      glib)
                      #bluetooth)

