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
include apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/depend.make

# Include the progress variables for this target.
include apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/progress.make

# Include the compile flags for this target's objects.
include apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/flags.make

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/flags.make
apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o: ../apps/two_way_audio/audio_server_mmap.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o -c /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_server_mmap.cpp

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.i"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_server_mmap.cpp > CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.i

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.s"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_server_mmap.cpp -o CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.s

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.requires:

.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.requires

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.provides: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.requires
	$(MAKE) -f apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/build.make apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.provides.build
.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.provides

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.provides.build: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o


apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/flags.make
apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o: ../apps/two_way_audio/audio_setting.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o -c /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_setting.cpp

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.i"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_setting.cpp > CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.i

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.s"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio/audio_setting.cpp -o CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.s

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.requires:

.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.requires

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.provides: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.requires
	$(MAKE) -f apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/build.make apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.provides.build
.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.provides

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.provides.build: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o


# Object files for target audio_server_mmap
audio_server_mmap_OBJECTS = \
"CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o" \
"CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o"

# External object files for target audio_server_mmap
audio_server_mmap_EXTERNAL_OBJECTS =

../bin/vienna/audio_server_mmap: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o
../bin/vienna/audio_server_mmap: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o
../bin/vienna/audio_server_mmap: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/build.make
../bin/vienna/audio_server_mmap: ../libs/vienna/libatk_audio_utils_mmap.so.1.0.0.0
../bin/vienna/audio_server_mmap: ../libs/vienna/libmsgbroker.so.1.0.0.0
../bin/vienna/audio_server_mmap: ../libs/vienna/libatk_encoder.a
../bin/vienna/audio_server_mmap: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../bin/vienna/audio_server_mmap"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/audio_server_mmap.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/build: ../bin/vienna/audio_server_mmap

.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/build

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/requires: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_server_mmap.cpp.o.requires
apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/requires: apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/audio_setting.cpp.o.requires

.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/requires

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/clean:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio && $(CMAKE_COMMAND) -P CMakeFiles/audio_server_mmap.dir/cmake_clean.cmake
.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/clean

apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/depend:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/two_way_audio /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/two_way_audio/CMakeFiles/audio_server_mmap.dir/depend

