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
include apps/playback_example/CMakeFiles/playback_example_mmap.dir/depend.make

# Include the progress variables for this target.
include apps/playback_example/CMakeFiles/playback_example_mmap.dir/progress.make

# Include the compile flags for this target's objects.
include apps/playback_example/CMakeFiles/playback_example_mmap.dir/flags.make

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o: apps/playback_example/CMakeFiles/playback_example_mmap.dir/flags.make
apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o: ../apps/playback_example/playback_example_mmap.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o -c /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/playback_example/playback_example_mmap.cpp

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.i"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/playback_example/playback_example_mmap.cpp > CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.i

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.s"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/playback_example/playback_example_mmap.cpp -o CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.s

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.requires:

.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.requires

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.provides: apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.requires
	$(MAKE) -f apps/playback_example/CMakeFiles/playback_example_mmap.dir/build.make apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.provides.build
.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.provides

apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.provides.build: apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o


# Object files for target playback_example_mmap
playback_example_mmap_OBJECTS = \
"CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o"

# External object files for target playback_example_mmap
playback_example_mmap_EXTERNAL_OBJECTS =

../bin/vienna/playback_example_mmap: apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o
../bin/vienna/playback_example_mmap: apps/playback_example/CMakeFiles/playback_example_mmap.dir/build.make
../bin/vienna/playback_example_mmap: ../libs/vienna/libatk_audio_utils_mmap.so.1.0.0.0
../bin/vienna/playback_example_mmap: ../libs/vienna/libmsgbroker.so.1.0.0.0
../bin/vienna/playback_example_mmap: ../libs/vienna/libatk_encoder.a
../bin/vienna/playback_example_mmap: apps/playback_example/CMakeFiles/playback_example_mmap.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/vienna/playback_example_mmap"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/playback_example_mmap.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
apps/playback_example/CMakeFiles/playback_example_mmap.dir/build: ../bin/vienna/playback_example_mmap

.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/build

apps/playback_example/CMakeFiles/playback_example_mmap.dir/requires: apps/playback_example/CMakeFiles/playback_example_mmap.dir/playback_example_mmap.cpp.o.requires

.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/requires

apps/playback_example/CMakeFiles/playback_example_mmap.dir/clean:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example && $(CMAKE_COMMAND) -P CMakeFiles/playback_example_mmap.dir/cmake_clean.cmake
.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/clean

apps/playback_example/CMakeFiles/playback_example_mmap.dir/depend:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/playback_example /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/playback_example/CMakeFiles/playback_example_mmap.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/playback_example/CMakeFiles/playback_example_mmap.dir/depend

