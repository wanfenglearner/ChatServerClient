# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/robin/workplace/cmake-3.22.0-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /home/robin/workplace/cmake-3.22.0-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/robin/projects/Chat

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/robin/projects/Chat/build

# Include any dependencies generated for this target.
include Client/CMakeFiles/Client.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include Client/CMakeFiles/Client.dir/compiler_depend.make

# Include the progress variables for this target.
include Client/CMakeFiles/Client.dir/progress.make

# Include the compile flags for this target's objects.
include Client/CMakeFiles/Client.dir/flags.make

Client/CMakeFiles/Client.dir/src/chatclient.cpp.o: Client/CMakeFiles/Client.dir/flags.make
Client/CMakeFiles/Client.dir/src/chatclient.cpp.o: ../Client/src/chatclient.cpp
Client/CMakeFiles/Client.dir/src/chatclient.cpp.o: Client/CMakeFiles/Client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robin/projects/Chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object Client/CMakeFiles/Client.dir/src/chatclient.cpp.o"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT Client/CMakeFiles/Client.dir/src/chatclient.cpp.o -MF CMakeFiles/Client.dir/src/chatclient.cpp.o.d -o CMakeFiles/Client.dir/src/chatclient.cpp.o -c /home/robin/projects/Chat/Client/src/chatclient.cpp

Client/CMakeFiles/Client.dir/src/chatclient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Client.dir/src/chatclient.cpp.i"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robin/projects/Chat/Client/src/chatclient.cpp > CMakeFiles/Client.dir/src/chatclient.cpp.i

Client/CMakeFiles/Client.dir/src/chatclient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Client.dir/src/chatclient.cpp.s"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robin/projects/Chat/Client/src/chatclient.cpp -o CMakeFiles/Client.dir/src/chatclient.cpp.s

Client/CMakeFiles/Client.dir/src/main.cpp.o: Client/CMakeFiles/Client.dir/flags.make
Client/CMakeFiles/Client.dir/src/main.cpp.o: ../Client/src/main.cpp
Client/CMakeFiles/Client.dir/src/main.cpp.o: Client/CMakeFiles/Client.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/robin/projects/Chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object Client/CMakeFiles/Client.dir/src/main.cpp.o"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT Client/CMakeFiles/Client.dir/src/main.cpp.o -MF CMakeFiles/Client.dir/src/main.cpp.o.d -o CMakeFiles/Client.dir/src/main.cpp.o -c /home/robin/projects/Chat/Client/src/main.cpp

Client/CMakeFiles/Client.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Client.dir/src/main.cpp.i"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/robin/projects/Chat/Client/src/main.cpp > CMakeFiles/Client.dir/src/main.cpp.i

Client/CMakeFiles/Client.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Client.dir/src/main.cpp.s"
	cd /home/robin/projects/Chat/build/Client && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/robin/projects/Chat/Client/src/main.cpp -o CMakeFiles/Client.dir/src/main.cpp.s

# Object files for target Client
Client_OBJECTS = \
"CMakeFiles/Client.dir/src/chatclient.cpp.o" \
"CMakeFiles/Client.dir/src/main.cpp.o"

# External object files for target Client
Client_EXTERNAL_OBJECTS =

../bin/Client: Client/CMakeFiles/Client.dir/src/chatclient.cpp.o
../bin/Client: Client/CMakeFiles/Client.dir/src/main.cpp.o
../bin/Client: Client/CMakeFiles/Client.dir/build.make
../bin/Client: Client/CMakeFiles/Client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/robin/projects/Chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../bin/Client"
	cd /home/robin/projects/Chat/build/Client && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
Client/CMakeFiles/Client.dir/build: ../bin/Client
.PHONY : Client/CMakeFiles/Client.dir/build

Client/CMakeFiles/Client.dir/clean:
	cd /home/robin/projects/Chat/build/Client && $(CMAKE_COMMAND) -P CMakeFiles/Client.dir/cmake_clean.cmake
.PHONY : Client/CMakeFiles/Client.dir/clean

Client/CMakeFiles/Client.dir/depend:
	cd /home/robin/projects/Chat/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/robin/projects/Chat /home/robin/projects/Chat/Client /home/robin/projects/Chat/build /home/robin/projects/Chat/build/Client /home/robin/projects/Chat/build/Client/CMakeFiles/Client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : Client/CMakeFiles/Client.dir/depend

