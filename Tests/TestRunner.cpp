#include "TestRunner.h"

#include <iostream>

namespace profiler_tests {

juce::String requestedTestCase;
bool quietOutput = false;

class ConsoleUnitTestRunner : public juce::UnitTestRunner {
   protected:
    void logMessage(const juce::String& message) override {
        if (!quietOutput) {
            std::cout << message << '\n';
        }
    }
};

}  // namespace profiler_tests

int main(int argc, char* argv[]) {
    juce::MessageManager::getInstance();

    for (int i = 1; i < argc; ++i) {
        const juce::String argument(argv[i]);
        if (argument == "--case" && i + 1 < argc) {
            profiler_tests::requestedTestCase = argv[++i];
        } else if (argument == "--quiet") {
            profiler_tests::quietOutput = true;
        }
    }

    const auto profilerTests = juce::UnitTest::getTestsInCategory("Profiler");
    profiler_tests::ConsoleUnitTestRunner runner;
    runner.runTests(profilerTests);

    int failures = 0;
    int passes = 0;
    for (int i = 0; i < runner.getNumResults(); ++i) {
        if (const auto* result = runner.getResult(i)) {
            failures += result->failures;
            passes += result->passes;
        }
    }

    std::cout << "Profiler test summary: "
              << profilerTests.size() << " suites, "
              << runner.getNumResults() << " test cases, "
              << (passes + failures) << " checks, "
              << passes << " passed, "
              << failures << " failed."
              << '\n';

    if (profiler_tests::requestedTestCase.isNotEmpty() && runner.getNumResults() == 0) {
        std::cout << "No Profiler test case matched: " << profiler_tests::requestedTestCase << '\n';
        return 1;
    }

    return failures == 0 ? 0 : 1;
}
