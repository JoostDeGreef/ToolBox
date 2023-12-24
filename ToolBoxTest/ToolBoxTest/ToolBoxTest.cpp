#include <stdio.h>

#include "gtest/gtest.h"
#include "gtest/ConsoleOutput.h"

GTEST_API_ int main(int argc, char** argv)
{
    ConsoleOutput::Start();
    printf("Running main() from ToolBoxTest.cpp\n");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
