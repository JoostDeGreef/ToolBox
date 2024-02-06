#ifndef VISUALSTUDIO_H
#define VISUALSTUDIO_H 1

#include "gtest.h"

class VisualStudioOutputData;
class VisualStudioOutput : public ::testing::EmptyTestEventListener 
{
private:
	class VisualStudioOutputData *m_data;

public:
	static void Start();

	VisualStudioOutput();
	~VisualStudioOutput();

    virtual void OnTestProgramStart(const ::testing::UnitTest& unit_test);
    virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test);
    virtual void OnEnvironmentsSetUpEnd(const ::testing::UnitTest& unit_test);
    virtual void OnEnvironmentsTearDownEnd(const ::testing::UnitTest& unit_test);
    virtual void OnTestCaseStart(const ::testing::TestCase& test_case);
    virtual void OnTestCaseEnd(const ::testing::TestCase& test_case);
	virtual void OnTestStart(const ::testing::TestInfo& test_info);
	virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result);
	virtual void OnTestEnd(const ::testing::TestInfo& test_info);
};  

#endif // VISUALSTUDIO_H
