#This setting is important
set(CMAKE_SYSTEM_NAME   Linux)

#setting compiler path
set(CMAKE_SYSTEM_VERSION 1)
set(MY_TOOL_PATH /opt/vtcs_toolchain/vienna/usr)
set(MY_TOOL_BIN_PATH /opt/vtcs_toolchain/vienna/usr/bin)
set(MY_CROSS_COMPILER arm-linux-)

#specify the cross compiler
set(CMAKE_C_COMPILER        ${MY_TOOL_BIN_PATH}/${MY_CROSS_COMPILER}gcc)
set(CMAKE_CXX_COMPILER   ${MY_TOOL_BIN_PATH}/${MY_CROSS_COMPILER}g++)

#where is the target environment
set(CMAKE_FIND_ROOT_PATH  ${MY_TOOL_PATH}/arm-buildroot-linux-uclibcgnueabihf/sysroot   ${MT_TOOL_PATH}/arm-buildroot-linux-uclibcgnueabihf )

#search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM  NEVER)

#for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY  ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)