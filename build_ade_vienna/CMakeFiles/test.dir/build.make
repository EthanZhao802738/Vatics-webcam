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
CMAKE_SOURCE_DIR = /home/ethan/Vatics-M5s

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ethan/Vatics-M5s/build_ade_vienna

# Include any dependencies generated for this target.
include CMakeFiles/test.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test.dir/flags.make

CMakeFiles/test.dir/src/adlr064.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/adlr064.cpp.o: ../src/adlr064.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/test.dir/src/adlr064.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/adlr064.cpp.o -c /home/ethan/Vatics-M5s/src/adlr064.cpp

CMakeFiles/test.dir/src/adlr064.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/adlr064.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/adlr064.cpp > CMakeFiles/test.dir/src/adlr064.cpp.i

CMakeFiles/test.dir/src/adlr064.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/adlr064.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/adlr064.cpp -o CMakeFiles/test.dir/src/adlr064.cpp.s

CMakeFiles/test.dir/src/adlr064.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/adlr064.cpp.o.requires

CMakeFiles/test.dir/src/adlr064.cpp.o.provides: CMakeFiles/test.dir/src/adlr064.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/adlr064.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/adlr064.cpp.o.provides

CMakeFiles/test.dir/src/adlr064.cpp.o.provides.build: CMakeFiles/test.dir/src/adlr064.cpp.o


CMakeFiles/test.dir/src/dbm.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/dbm.cpp.o: ../src/dbm.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/test.dir/src/dbm.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/dbm.cpp.o -c /home/ethan/Vatics-M5s/src/dbm.cpp

CMakeFiles/test.dir/src/dbm.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/dbm.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/dbm.cpp > CMakeFiles/test.dir/src/dbm.cpp.i

CMakeFiles/test.dir/src/dbm.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/dbm.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/dbm.cpp -o CMakeFiles/test.dir/src/dbm.cpp.s

CMakeFiles/test.dir/src/dbm.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/dbm.cpp.o.requires

CMakeFiles/test.dir/src/dbm.cpp.o.provides: CMakeFiles/test.dir/src/dbm.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/dbm.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/dbm.cpp.o.provides

CMakeFiles/test.dir/src/dbm.cpp.o.provides.build: CMakeFiles/test.dir/src/dbm.cpp.o


CMakeFiles/test.dir/src/main.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/test.dir/src/main.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/main.cpp.o -c /home/ethan/Vatics-M5s/src/main.cpp

CMakeFiles/test.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/main.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/main.cpp > CMakeFiles/test.dir/src/main.cpp.i

CMakeFiles/test.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/main.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/main.cpp -o CMakeFiles/test.dir/src/main.cpp.s

CMakeFiles/test.dir/src/main.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/main.cpp.o.requires

CMakeFiles/test.dir/src/main.cpp.o.provides: CMakeFiles/test.dir/src/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/main.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/main.cpp.o.provides

CMakeFiles/test.dir/src/main.cpp.o.provides.build: CMakeFiles/test.dir/src/main.cpp.o


CMakeFiles/test.dir/src/net.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/net.cpp.o: ../src/net.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/test.dir/src/net.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/net.cpp.o -c /home/ethan/Vatics-M5s/src/net.cpp

CMakeFiles/test.dir/src/net.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/net.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/net.cpp > CMakeFiles/test.dir/src/net.cpp.i

CMakeFiles/test.dir/src/net.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/net.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/net.cpp -o CMakeFiles/test.dir/src/net.cpp.s

CMakeFiles/test.dir/src/net.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/net.cpp.o.requires

CMakeFiles/test.dir/src/net.cpp.o.provides: CMakeFiles/test.dir/src/net.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/net.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/net.cpp.o.provides

CMakeFiles/test.dir/src/net.cpp.o.provides.build: CMakeFiles/test.dir/src/net.cpp.o


CMakeFiles/test.dir/src/spi.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/spi.cpp.o: ../src/spi.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/test.dir/src/spi.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/spi.cpp.o -c /home/ethan/Vatics-M5s/src/spi.cpp

CMakeFiles/test.dir/src/spi.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/spi.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/spi.cpp > CMakeFiles/test.dir/src/spi.cpp.i

CMakeFiles/test.dir/src/spi.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/spi.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/spi.cpp -o CMakeFiles/test.dir/src/spi.cpp.s

CMakeFiles/test.dir/src/spi.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/spi.cpp.o.requires

CMakeFiles/test.dir/src/spi.cpp.o.provides: CMakeFiles/test.dir/src/spi.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/spi.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/spi.cpp.o.provides

CMakeFiles/test.dir/src/spi.cpp.o.provides.build: CMakeFiles/test.dir/src/spi.cpp.o


CMakeFiles/test.dir/src/ulis_encode.cpp.o: CMakeFiles/test.dir/flags.make
CMakeFiles/test.dir/src/ulis_encode.cpp.o: ../src/ulis_encode.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/test.dir/src/ulis_encode.cpp.o"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test.dir/src/ulis_encode.cpp.o -c /home/ethan/Vatics-M5s/src/ulis_encode.cpp

CMakeFiles/test.dir/src/ulis_encode.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test.dir/src/ulis_encode.cpp.i"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ethan/Vatics-M5s/src/ulis_encode.cpp > CMakeFiles/test.dir/src/ulis_encode.cpp.i

CMakeFiles/test.dir/src/ulis_encode.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test.dir/src/ulis_encode.cpp.s"
	/opt/vtcs_toolchain/vienna/usr/bin/arm-linux-g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ethan/Vatics-M5s/src/ulis_encode.cpp -o CMakeFiles/test.dir/src/ulis_encode.cpp.s

CMakeFiles/test.dir/src/ulis_encode.cpp.o.requires:

.PHONY : CMakeFiles/test.dir/src/ulis_encode.cpp.o.requires

CMakeFiles/test.dir/src/ulis_encode.cpp.o.provides: CMakeFiles/test.dir/src/ulis_encode.cpp.o.requires
	$(MAKE) -f CMakeFiles/test.dir/build.make CMakeFiles/test.dir/src/ulis_encode.cpp.o.provides.build
.PHONY : CMakeFiles/test.dir/src/ulis_encode.cpp.o.provides

CMakeFiles/test.dir/src/ulis_encode.cpp.o.provides.build: CMakeFiles/test.dir/src/ulis_encode.cpp.o


# Object files for target test
test_OBJECTS = \
"CMakeFiles/test.dir/src/adlr064.cpp.o" \
"CMakeFiles/test.dir/src/dbm.cpp.o" \
"CMakeFiles/test.dir/src/main.cpp.o" \
"CMakeFiles/test.dir/src/net.cpp.o" \
"CMakeFiles/test.dir/src/spi.cpp.o" \
"CMakeFiles/test.dir/src/ulis_encode.cpp.o"

# External object files for target test
test_EXTERNAL_OBJECTS =

../bin/test: CMakeFiles/test.dir/src/adlr064.cpp.o
../bin/test: CMakeFiles/test.dir/src/dbm.cpp.o
../bin/test: CMakeFiles/test.dir/src/main.cpp.o
../bin/test: CMakeFiles/test.dir/src/net.cpp.o
../bin/test: CMakeFiles/test.dir/src/spi.cpp.o
../bin/test: CMakeFiles/test.dir/src/ulis_encode.cpp.o
../bin/test: CMakeFiles/test.dir/build.make
../bin/test: CMakeFiles/test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable ../bin/test"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test.dir/build: ../bin/test

.PHONY : CMakeFiles/test.dir/build

CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/adlr064.cpp.o.requires
CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/dbm.cpp.o.requires
CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/main.cpp.o.requires
CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/net.cpp.o.requires
CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/spi.cpp.o.requires
CMakeFiles/test.dir/requires: CMakeFiles/test.dir/src/ulis_encode.cpp.o.requires

.PHONY : CMakeFiles/test.dir/requires

CMakeFiles/test.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test.dir/clean

CMakeFiles/test.dir/depend:
	cd /home/ethan/Vatics-M5s/build_ade_vienna && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ethan/Vatics-M5s /home/ethan/Vatics-M5s /home/ethan/Vatics-M5s/build_ade_vienna /home/ethan/Vatics-M5s/build_ade_vienna /home/ethan/Vatics-M5s/build_ade_vienna/CMakeFiles/test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test.dir/depend

