# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

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
CMAKE_COMMAND = /home/lorre851/CLion/bin/cmake/bin/cmake

# The command to remove a file.
RM = /home/lorre851/CLion/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/lorre851/CLionProjects/trainframe_emu

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/trainframe_emu.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/trainframe_emu.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/trainframe_emu.dir/flags.make

CMakeFiles/trainframe_emu.dir/src/main.cpp.o: CMakeFiles/trainframe_emu.dir/flags.make
CMakeFiles/trainframe_emu.dir/src/main.cpp.o: ../src/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/trainframe_emu.dir/src/main.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trainframe_emu.dir/src/main.cpp.o -c /home/lorre851/CLionProjects/trainframe_emu/src/main.cpp

CMakeFiles/trainframe_emu.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trainframe_emu.dir/src/main.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lorre851/CLionProjects/trainframe_emu/src/main.cpp > CMakeFiles/trainframe_emu.dir/src/main.cpp.i

CMakeFiles/trainframe_emu.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trainframe_emu.dir/src/main.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lorre851/CLionProjects/trainframe_emu/src/main.cpp -o CMakeFiles/trainframe_emu.dir/src/main.cpp.s

CMakeFiles/trainframe_emu.dir/src/main.cpp.o.requires:

.PHONY : CMakeFiles/trainframe_emu.dir/src/main.cpp.o.requires

CMakeFiles/trainframe_emu.dir/src/main.cpp.o.provides: CMakeFiles/trainframe_emu.dir/src/main.cpp.o.requires
	$(MAKE) -f CMakeFiles/trainframe_emu.dir/build.make CMakeFiles/trainframe_emu.dir/src/main.cpp.o.provides.build
.PHONY : CMakeFiles/trainframe_emu.dir/src/main.cpp.o.provides

CMakeFiles/trainframe_emu.dir/src/main.cpp.o.provides.build: CMakeFiles/trainframe_emu.dir/src/main.cpp.o


CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o: CMakeFiles/trainframe_emu.dir/flags.make
CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o: ../src/interfaces.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o -c /home/lorre851/CLionProjects/trainframe_emu/src/interfaces.cpp

CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lorre851/CLionProjects/trainframe_emu/src/interfaces.cpp > CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.i

CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lorre851/CLionProjects/trainframe_emu/src/interfaces.cpp -o CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.s

CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.requires:

.PHONY : CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.requires

CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.provides: CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.requires
	$(MAKE) -f CMakeFiles/trainframe_emu.dir/build.make CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.provides.build
.PHONY : CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.provides

CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.provides.build: CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o


CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o: CMakeFiles/trainframe_emu.dir/flags.make
CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o: ../src/depthpeopledetector.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o -c /home/lorre851/CLionProjects/trainframe_emu/src/depthpeopledetector.cpp

CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lorre851/CLionProjects/trainframe_emu/src/depthpeopledetector.cpp > CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.i

CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lorre851/CLionProjects/trainframe_emu/src/depthpeopledetector.cpp -o CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.s

CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.requires:

.PHONY : CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.requires

CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.provides: CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.requires
	$(MAKE) -f CMakeFiles/trainframe_emu.dir/build.make CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.provides.build
.PHONY : CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.provides

CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.provides.build: CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o


CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o: CMakeFiles/trainframe_emu.dir/flags.make
CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o: ../src/zeromq_tools.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o -c /home/lorre851/CLionProjects/trainframe_emu/src/zeromq_tools.cpp

CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lorre851/CLionProjects/trainframe_emu/src/zeromq_tools.cpp > CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.i

CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lorre851/CLionProjects/trainframe_emu/src/zeromq_tools.cpp -o CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.s

CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.requires:

.PHONY : CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.requires

CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.provides: CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.requires
	$(MAKE) -f CMakeFiles/trainframe_emu.dir/build.make CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.provides.build
.PHONY : CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.provides

CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.provides.build: CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o


CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o: CMakeFiles/trainframe_emu.dir/flags.make
CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o: ../src/cvclient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o -c /home/lorre851/CLionProjects/trainframe_emu/src/cvclient.cpp

CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/lorre851/CLionProjects/trainframe_emu/src/cvclient.cpp > CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.i

CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/lorre851/CLionProjects/trainframe_emu/src/cvclient.cpp -o CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.s

CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.requires:

.PHONY : CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.requires

CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.provides: CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.requires
	$(MAKE) -f CMakeFiles/trainframe_emu.dir/build.make CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.provides.build
.PHONY : CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.provides

CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.provides.build: CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o


# Object files for target trainframe_emu
trainframe_emu_OBJECTS = \
"CMakeFiles/trainframe_emu.dir/src/main.cpp.o" \
"CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o" \
"CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o" \
"CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o" \
"CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o"

# External object files for target trainframe_emu
trainframe_emu_EXTERNAL_OBJECTS =

trainframe_emu: CMakeFiles/trainframe_emu.dir/src/main.cpp.o
trainframe_emu: CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o
trainframe_emu: CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o
trainframe_emu: CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o
trainframe_emu: CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o
trainframe_emu: CMakeFiles/trainframe_emu.dir/build.make
trainframe_emu: CMakeFiles/trainframe_emu.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable trainframe_emu"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/trainframe_emu.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/trainframe_emu.dir/build: trainframe_emu

.PHONY : CMakeFiles/trainframe_emu.dir/build

CMakeFiles/trainframe_emu.dir/requires: CMakeFiles/trainframe_emu.dir/src/main.cpp.o.requires
CMakeFiles/trainframe_emu.dir/requires: CMakeFiles/trainframe_emu.dir/src/interfaces.cpp.o.requires
CMakeFiles/trainframe_emu.dir/requires: CMakeFiles/trainframe_emu.dir/src/depthpeopledetector.cpp.o.requires
CMakeFiles/trainframe_emu.dir/requires: CMakeFiles/trainframe_emu.dir/src/zeromq_tools.cpp.o.requires
CMakeFiles/trainframe_emu.dir/requires: CMakeFiles/trainframe_emu.dir/src/cvclient.cpp.o.requires

.PHONY : CMakeFiles/trainframe_emu.dir/requires

CMakeFiles/trainframe_emu.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/trainframe_emu.dir/cmake_clean.cmake
.PHONY : CMakeFiles/trainframe_emu.dir/clean

CMakeFiles/trainframe_emu.dir/depend:
	cd /home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/lorre851/CLionProjects/trainframe_emu /home/lorre851/CLionProjects/trainframe_emu /home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug /home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug /home/lorre851/CLionProjects/trainframe_emu/cmake-build-debug/CMakeFiles/trainframe_emu.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/trainframe_emu.dir/depend
