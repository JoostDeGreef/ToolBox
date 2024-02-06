# CMakeList.txt : CMake project for ToolBoxTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
#else()
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#endif()

# Add source to this project's executable.
add_executable (
    ToolBoxTest 
    "ToolBoxTest.cpp"

    "test/BinaryTest.cpp"
    "test/FIFOTest.cpp"
    "test/GenericTest.cpp"
    "test/ThreadPoolTest.cpp"

    "include/binary.h"
    "include/binary/BinaryT.h"
    "include/binary/BinaryData.h"
    "include/binary/BinaryDataOperations.h"
    "include/FIFO_LockLess.h"
    "include/FIFO_Mutex.h"
    "include/FIFO.h"
    "include/FIFO_NoLock.h"
    "include/ThreadPool_Mutex.h"
    "include/ThreadPool.h"
    "include/ToolBox.h"
   
    "gtest/ConsoleOutput.cpp"
    "gtest/ConsoleOutput.h"
    "gtest/gtest.h"
    "gtest/gtest-all.cc"
    "gtest/VisualStudioOutput.cpp"
    "gtest/VisualStudioOutput.h")

# TODO: Add tests and install targets if needed.
