# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

if("/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz" STREQUAL "")
  message(FATAL_ERROR "LOCAL can't be empty")
endif()

if(NOT EXISTS "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz")
  message(FATAL_ERROR "File not found: /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz")
endif()

if("" STREQUAL "")
  message(WARNING "File will not be verified since no URL_HASH specified")
  return()
endif()

if("" STREQUAL "")
  message(FATAL_ERROR "EXPECT_VALUE can't be empty")
endif()

message(STATUS "verifying file...
     file='/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz'")

file("" "/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz" actual_value)

if(NOT "${actual_value}" STREQUAL "")
  message(FATAL_ERROR "error:  hash of
  /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/open_source_3rd/opencore-amr/opencore-amr-0.1.3.tar.gz
does not match expected value
  expected: ''
    actual: '${actual_value}'
")
endif()

message(STATUS "verifying file... done")
