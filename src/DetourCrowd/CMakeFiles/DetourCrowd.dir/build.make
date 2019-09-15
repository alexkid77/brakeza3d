# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.14

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
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd

# Include any dependencies generated for this target.
include CMakeFiles/DetourCrowd.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/DetourCrowd.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/DetourCrowd.dir/flags.make

CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o: Source/DetourCrowd.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourCrowd.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourCrowd.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.i

CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourCrowd.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.s

CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o: Source/DetourLocalBoundary.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourLocalBoundary.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourLocalBoundary.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.i

CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourLocalBoundary.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.s

CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o: Source/DetourObstacleAvoidance.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourObstacleAvoidance.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourObstacleAvoidance.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.i

CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourObstacleAvoidance.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.s

CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o: Source/DetourPathCorridor.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathCorridor.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathCorridor.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.i

CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathCorridor.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.s

CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o: Source/DetourPathQueue.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathQueue.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathQueue.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.i

CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourPathQueue.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.s

CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o: CMakeFiles/DetourCrowd.dir/flags.make
CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o: Source/DetourProximityGrid.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o"
	/Library/Developer/CommandLineTools/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o -c /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourProximityGrid.cpp

CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourProximityGrid.cpp > CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.i

CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/Source/DetourProximityGrid.cpp -o CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.s

# Object files for target DetourCrowd
DetourCrowd_OBJECTS = \
"CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o" \
"CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o" \
"CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o" \
"CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o" \
"CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o" \
"CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o"

# External object files for target DetourCrowd
DetourCrowd_EXTERNAL_OBJECTS =

libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourCrowd.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourLocalBoundary.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourObstacleAvoidance.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourPathCorridor.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourPathQueue.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/Source/DetourProximityGrid.o
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/build.make
libDetourCrowd.VERSION.dylib: CMakeFiles/DetourCrowd.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX shared library libDetourCrowd.dylib"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DetourCrowd.dir/link.txt --verbose=$(VERBOSE)
	$(CMAKE_COMMAND) -E cmake_symlink_library libDetourCrowd.VERSION.dylib libDetourCrowd.VERSION.dylib libDetourCrowd.dylib

libDetourCrowd.dylib: libDetourCrowd.VERSION.dylib
	@$(CMAKE_COMMAND) -E touch_nocreate libDetourCrowd.dylib

# Rule to build all files generated by this target.
CMakeFiles/DetourCrowd.dir/build: libDetourCrowd.dylib

.PHONY : CMakeFiles/DetourCrowd.dir/build

CMakeFiles/DetourCrowd.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/DetourCrowd.dir/cmake_clean.cmake
.PHONY : CMakeFiles/DetourCrowd.dir/clean

CMakeFiles/DetourCrowd.dir/depend:
	cd /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd /Users/darkhead/CLionProjects/recastnavigation/DetourCrowd/CMakeFiles/DetourCrowd.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/DetourCrowd.dir/depend

