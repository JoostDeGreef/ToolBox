#include <stdio.h>

#include "gtest/gtest.h"
#include "gtest/ConsoleOutput.h"
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include "gtest/VisualStudioOutput.h"
#endif

GTEST_API_ int main(int argc, char** argv)
{
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    VisualStudioOutput::Start();
#endif
    ConsoleOutput::Start();
    printf("Running main() from GoogleTestMain.cpp\n");
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
