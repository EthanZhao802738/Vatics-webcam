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
include apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/depend.make

# Include the progress variables for this target.
include apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/progress.make

# Include the compile flags for this target's objects.
include apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/flags.make

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/flags.make
apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o: ../apps/volume_ctrl/volume_ctrl.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o -c /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/volume_ctrl/volume_ctrl.cpp

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.i"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/volume_ctrl/volume_ctrl.cpp > CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.i

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.s"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl && /opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/volume_ctrl/volume_ctrl.cpp -o CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.s

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.requires:

.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.requires

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.provides: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.requires
	$(MAKE) -f apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/build.make apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.provides.build
.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.provides

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.provides.build: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o


# Object files for target volume_ctrl
volume_ctrl_OBJECTS = \
"CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o"

# External object files for target volume_ctrl
volume_ctrl_EXTERNAL_OBJECTS =

../bin/vienna/volume_ctrl: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o
../bin/vienna/volume_ctrl: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/build.make
../bin/vienna/volume_ctrl: ../libs/vienna/libatk_audio_utils_mmap.so.1.0.0.0
../bin/vienna/volume_ctrl: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/vienna/volume_ctrl"
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/volume_ctrl.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/build: ../bin/vienna/volume_ctrl

.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/build

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/requires: apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/volume_ctrl.cpp.o.requires

.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/requires

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/clean:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl && $(CMAKE_COMMAND) -P CMakeFiles/volume_ctrl.dir/cmake_clean.cmake
.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/clean

apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/depend:
	cd /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/apps/volume_ctrl /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl /home/ethan/M5S-V2.3/03_SDK/02_Software_Tool_Kit/sdk_v2.3/sdk/modules/build_vienna/apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : apps/volume_ctrl/CMakeFiles/volume_ctrl.dir/depend
