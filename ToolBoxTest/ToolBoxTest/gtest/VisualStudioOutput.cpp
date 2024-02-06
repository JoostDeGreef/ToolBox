#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include <vector>
#include <string>

#include "STDARG.H"
#include "Windows.h"

#include "VisualStudioOutput.h"

using namespace ::testing;
using namespace std;

namespace 
{
	void OutputFormattedDebugStringA(const char *format, ...)
	{
		va_list args;
		va_start( args, format );
		int len = _vscprintf( format, args ) + 1;
		char *str = new char[len * sizeof(char)];
		vsprintf_s(str, len, format, args );
		OutputDebugStringA(str);
		delete [] str;
	}
};

class VisualStudioOutputData 
{
private:
	LARGE_INTEGER m_lastTestStart;
	LARGE_INTEGER m_frequency;
	double m_duration;

public:
	std::vector<std::vector<std::string> *> m_failureLines;
	std::vector<std::string> *m_failuresCurrentTestCase;

	VisualStudioOutputData()
	{
		QueryPerformanceFrequency(&m_frequency);
	}
	~VisualStudioOutputData()
	{
		for(auto iter=m_failureLines.begin();iter!=m_failureLines.end();++iter)
		{
			delete *iter;
		}
		m_failureLines.clear();
	}

	void PrintFailedTests()
	{
		for(auto iter=m_failureLines.cbegin();iter!=m_failureLines.cend();++iter)
		{
			for(auto line=(*iter)->cbegin();line != (*iter)->cend(); ++line)
			{
				OutputFormattedDebugStringA((*line + "\n").c_str());
			}
		}
	}

	void StartTestCase(std::string name)
	{
		m_failuresCurrentTestCase = new std::vector<std::string>;
		m_failuresCurrentTestCase->push_back("[" + name + "]");
	}

	void AddTestCaseFailure(std::string failure)
	{
		m_failuresCurrentTestCase->push_back(failure);
	}

	void EndTestCase()
	{
		if( m_failuresCurrentTestCase->size()>1 )
		{
			m_failureLines.push_back(m_failuresCurrentTestCase);
		}
		else
		{
			delete m_failuresCurrentTestCase;
		}
	}

	void StartTimer()
	{
		QueryPerformanceCounter(&m_lastTestStart);
		m_duration = 0;
	}

	unsigned long long GetTimerDurationMS()
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		double duration = (1000.0*(now.QuadPart - m_lastTestStart.QuadPart))/m_frequency.QuadPart;
		double res = duration - m_duration;
		m_duration = duration;
		return (unsigned long long)res;
	}
};

void VisualStudioOutput::Start()
{
    if( IsDebuggerPresent() )
	{
		::testing::UnitTest& unit_test = *::testing::UnitTest::GetInstance();
		::testing::TestEventListeners& listeners = unit_test.listeners();
		listeners.Append(new VisualStudioOutput);
	}
}

VisualStudioOutput::VisualStudioOutput()
{
	m_data = new VisualStudioOutputData();
}

VisualStudioOutput::~VisualStudioOutput()
{
	delete m_data;
}

// Fired before any test activity starts.
void VisualStudioOutput::OnTestProgramStart(const UnitTest& unit_test)
{
	OutputFormattedDebugStringA("[==========] Running %i tests from %i test cases.\n", unit_test.test_to_run_count(), unit_test.test_case_to_run_count());
}

// Fired after all test activities have ended.
void VisualStudioOutput::OnTestProgramEnd(const UnitTest& unit_test) 
{
	OutputFormattedDebugStringA("[==========] %i tests from %i test cases ran (%llu ms total).\n", unit_test.test_to_run_count(), unit_test.test_case_to_run_count(), unit_test.elapsed_time());
	if( unit_test.failed_test_count() > 0 )
	{
		OutputFormattedDebugStringA("[  PASSED  ] %i tests.\n", unit_test.successful_test_count());
	}
	if( unit_test.disabled_test_count() > 0 )
	{
		OutputFormattedDebugStringA("[ DISABLED ] %i tests.\n", unit_test.disabled_test_count());
	}
	if( unit_test.failed_test_count() > 0 )
	{
		OutputFormattedDebugStringA("[  FAILED  ] %i tests from %i test cases, listed below:\n", unit_test.failed_test_count(), unit_test.failed_test_case_count());
		m_data->PrintFailedTests();
	}
}

// Fired after environment set-up for each iteration of tests ends.
void VisualStudioOutput::OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/)
{
	OutputFormattedDebugStringA("[----------] Global test environment set-up.\n");
}

// Fired after environment tear-down for each iteration of tests ends.
void VisualStudioOutput::OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/)
{
	OutputFormattedDebugStringA("[----------] Global test environment tear-down.\n");
}

// Fired before the test case starts.
void VisualStudioOutput::OnTestCaseStart(const TestCase& test_case) 
{
	m_data->StartTestCase(test_case.name());
	OutputFormattedDebugStringA("[----------] %i tests from %s.\n", test_case.test_to_run_count(), test_case.name());
	m_data->StartTimer();
}

// Fired after the test case ends.
void VisualStudioOutput::OnTestCaseEnd(const TestCase& test_case) 
{
	OutputFormattedDebugStringA("[----------] %i tests from %s (%llu ms total).\n\n", test_case.test_to_run_count(), test_case.name(), test_case.elapsed_time());
	m_data->EndTestCase();
}

// Called before a test starts.
void VisualStudioOutput::OnTestStart(const TestInfo& test_info) 
{
	OutputFormattedDebugStringA("[ RUN      ] %s.%s.\n", test_info.test_case_name(), test_info.name());
}

// Called after a failed assertion or a SUCCEED() invocation.
void VisualStudioOutput::OnTestPartResult(const TestPartResult& test_part_result) 
{
	if( test_part_result.failed() )
	{
        const char *filename = test_part_result.file_name();
        if (nullptr != filename)
        {
            m_data->AddTestCaseFailure(filename + std::string("(") + std::to_string((unsigned long long)test_part_result.line_number()) + "): " + test_part_result.summary());
        }
        else
        {
            m_data->AddTestCaseFailure(test_part_result.summary());
        }
	}
    else
    {
        const char *message = test_part_result.message();
        const char *user = strchr(message, '\n');
        if(user != nullptr )
        {
            message = user + 1;
        }
        OutputFormattedDebugStringA("[    DEBUG ] %s\n", message);
    }
}

// Called after a test ends.
void VisualStudioOutput::OnTestEnd(const TestInfo& test_info) 
{
	unsigned long long duration = m_data->GetTimerDurationMS();
	if( test_info.result()->Passed() )
	{
		OutputFormattedDebugStringA("[       OK ] %s.%s (%llu ms).\n", test_info.test_case_name(), test_info.name(), duration);
	}
	else
	{
		OutputFormattedDebugStringA("[   FAILED ] %s.%s (%llu ms).\n", test_info.test_case_name(), test_info.name(), duration);
	}
}
#endif // WIN32