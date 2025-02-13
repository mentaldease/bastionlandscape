#ifndef UNITTEST_TESTRUNNER_H
#define UNITTEST_TESTRUNNER_H


namespace UnitTest {

class TestReporter;
class TestList;


int RunAllTests();
int RunAllTests(TestReporter& reporter, TestList const& list, int maxTestTimeInMs = 0);

}


#endif

