# Install script for directory: /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1.0.0.0"
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1"
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1.0.0.0;/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1;/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib" TYPE SHARED_LIBRARY FILES
    "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/libs/vienna/libmsgbroker.so.1.0.0.0"
    "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/libs/vienna/libmsgbroker.so.1"
    "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/libs/vienna/libmsgbroker.so"
    )
  foreach(file
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1.0.0.0"
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so.1"
      "$ENV{DESTDIR}/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/lib/libmsgbroker.so"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/include/msg_broker.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/../vtcs_root_vienna/include" TYPE FILE FILES "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker/msg_broker.h")
endif()

