#include <vector>
#include <string>
#include <iostream>

#include <cstdarg>

#include "ConsoleOutput.h"

using namespace ::testing;
using namespace std;

namespace 
{
	void OutputFormattedString(const char *format, ...)
	{
		va_list args;
		va_start( args, format );
		int len = vsnprintf( nullptr, 0, format, args ) + 1;
		char *str = new char[len * sizeof(char)];
		vsnprintf(str, len, format, args );
		std::cout << str << std::endl;
		delete [] str;
	}
};

class ConsoleOutputData 
{
public:
	std::vector<std::vector<std::string> *> m_failureLines;
	std::vector<std::string> *m_failuresCurrentTestCase;

	ConsoleOutputData()
	{
	}
	~ConsoleOutputData()
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
				OutputFormattedString((*line).c_str());
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
};

void ConsoleOutput::Start()
{
	::testing::UnitTest& unit_test = *::testing::UnitTest::GetInstance();
	::testing::TestEventListeners& listeners = unit_test.listeners();
	listeners.Append(new ConsoleOutput);
}

ConsoleOutput::ConsoleOutput()
{
	m_data = new ConsoleOutputData();
}

ConsoleOutput::~ConsoleOutput()
{
	delete m_data;
}

// Fired before any test activity starts.
void ConsoleOutput::OnTestProgramStart(const UnitTest& unit_test)
{}

// Fired after all test activities have ended.
void ConsoleOutput::OnTestProgramEnd(const UnitTest& unit_test) 
{}

// Fired after environment set-up for each iteration of tests ends.
void ConsoleOutput::OnEnvironmentsSetUpEnd(const UnitTest& /*unit_test*/)
{}

// Fired after environment tear-down for each iteration of tests ends.
void ConsoleOutput::OnEnvironmentsTearDownEnd(const UnitTest& /*unit_test*/)
{}

// Fired before the test case starts.
void ConsoleOutput::OnTestCaseStart(const TestCase& test_case) 
{}

// Fired after the test case ends.
void ConsoleOutput::OnTestCaseEnd(const TestCase& test_case) 
{}

// Called before a test starts.
void ConsoleOutput::OnTestStart(const TestInfo& test_info) 
{}

// Called after a failed assertion or a SUCCEED() invocation.
void ConsoleOutput::OnTestPartResult(const TestPartResult& test_part_result) 
{
	if( !test_part_result.failed() )
    {
        const char *message = test_part_result.message();
        const char *user = strchr(message, '\n');
        if(user != nullptr )
        {
            message = user + 1;
        }
        OutputFormattedString("[    DEBUG ] %s", message);
    }
}

// Called after a test ends.
void ConsoleOutput::OnTestEnd(const TestInfo& test_info) 
{}
