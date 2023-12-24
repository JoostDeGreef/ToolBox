# CMakeList.txt : CMake project for ToolBoxTest, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (ToolBoxTest "ToolBoxTest.cpp"  "gtest/gtest.h" "gtest/gtest-all.cc" "test/GenericTest.cpp" "include/ThreadPool_Mutex.h" "gtest/ConsoleOutput.cpp" "gtest/ConsoleOutput.h" "include/ToolBox.h" "include/ThreadPool.h" "test/ThreadPoolTest.cpp" "include/FIFO.h" "include/FIFO_NoLock.h" "test/FIFOTest.cpp" "include/FIFO_LockLess.h")

# TODO: Add tests and install targets if needed.
