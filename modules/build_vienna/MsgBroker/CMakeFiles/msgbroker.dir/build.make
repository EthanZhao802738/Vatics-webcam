# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna

# Include any dependencies generated for this target.
include MsgBroker/CMakeFiles/msgbroker.dir/depend.make

# Include the progress variables for this target.
include MsgBroker/CMakeFiles/msgbroker.dir/progress.make

# Include the compile flags for this target's objects.
include MsgBroker/CMakeFiles/msgbroker.dir/flags.make

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o: MsgBroker/CMakeFiles/msgbroker.dir/flags.make
MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o: ../MsgBroker/msg_broker.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/msgbroker.dir/msg_broker.c.o   -c /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker/msg_broker.c

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/msgbroker.dir/msg_broker.c.i"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker/msg_broker.c > CMakeFiles/msgbroker.dir/msg_broker.c.i

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/msgbroker.dir/msg_broker.c.s"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-gcc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker/msg_broker.c -o CMakeFiles/msgbroker.dir/msg_broker.c.s

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.requires:

.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.requires

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.provides: MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.requires
	$(MAKE) -f MsgBroker/CMakeFiles/msgbroker.dir/build.make MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.provides.build
.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.provides

MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.provides.build: MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o


# Object files for target msgbroker
msgbroker_OBJECTS = \
"CMakeFiles/msgbroker.dir/msg_broker.c.o"

# External object files for target msgbroker
msgbroker_EXTERNAL_OBJECTS =

../libs/vienna/libmsgbroker.so.1.0.0.0: MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o
../libs/vienna/libmsgbroker.so.1.0.0.0: MsgBroker/CMakeFiles/msgbroker.dir/build.make
../libs/vienna/libmsgbroker.so.1.0.0.0: MsgBroker/CMakeFiles/msgbroker.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library ../../libs/vienna/libmsgbroker.so"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/msgbroker.dir/link.txt --verbose=$(VERBOSE)
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && $(CMAKE_COMMAND) -E cmake_symlink_library ../../libs/vienna/libmsgbroker.so.1.0.0.0 ../../libs/vienna/libmsgbroker.so.1 ../../libs/vienna/libmsgbroker.so

../libs/vienna/libmsgbroker.so.1: ../libs/vienna/libmsgbroker.so.1.0.0.0
	@$(CMAKE_COMMAND) -E touch_nocreate ../libs/vienna/libmsgbroker.so.1

../libs/vienna/libmsgbroker.so: ../libs/vienna/libmsgbroker.so.1.0.0.0
	@$(CMAKE_COMMAND) -E touch_nocreate ../libs/vienna/libmsgbroker.so

# Rule to build all files generated by this target.
MsgBroker/CMakeFiles/msgbroker.dir/build: ../libs/vienna/libmsgbroker.so

.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/build

MsgBroker/CMakeFiles/msgbroker.dir/requires: MsgBroker/CMakeFiles/msgbroker.dir/msg_broker.c.o.requires

.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/requires

MsgBroker/CMakeFiles/msgbroker.dir/clean:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker && $(CMAKE_COMMAND) -P CMakeFiles/msgbroker.dir/cmake_clean.cmake
.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/clean

MsgBroker/CMakeFiles/msgbroker.dir/depend:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/MsgBroker /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/MsgBroker/CMakeFiles/msgbroker.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : MsgBroker/CMakeFiles/msgbroker.dir/depend

