#import "config.h"

#import "progress_listener.h"

#import <algorithm>
#import <iostream>
#import <iterator>
#import <numeric>
#import <stdexcept>
#import <cppunit/TestCase.h>
#import <cppunit/TestSuite.h>
#import "torrent/utils/log.h"
#import "torrent/utils/log_buffer.h"

static std::string
get_test_path(const test_list_type& tl) {
  if (tl.size() < 3)
    return "";

  return std::accumulate(std::next(std::next(tl.begin())), std::prev(tl.end()), std::string(), [](std::string result, CppUnit::Test* test) {
      return std::move(result) + test->getName() + "::";
    });
}

static int
get_suite_count(CppUnit::Test *suite) {
  auto test_suite = dynamic_cast<CppUnit::TestSuite*>(suite);
  if (test_suite == nullptr)
    return 0;

  return std::accumulate(test_suite->getTests().begin(), test_suite->getTests().end(), 0, [](int result, CppUnit::Test* test) {
      return result + (dynamic_cast<CppUnit::TestSuite*>(test) != nullptr);
    });
}

static int
get_case_count(CppUnit::Test *suite) {
  auto test_suite = dynamic_cast<CppUnit::TestSuite*>(suite);
  if (test_suite == nullptr)
    return 0;

  return std::accumulate(test_suite->getTests().begin(), test_suite->getTests().end(), 0, [](int result, CppUnit::Test* test) {
      return result + (dynamic_cast<CppUnit::TestCase*>(test) != nullptr);
    });
}

static int
get_sub_case_count(CppUnit::Test *suite) {
  auto test_suite = dynamic_cast<CppUnit::TestSuite*>(suite);
  if (test_suite == nullptr)
    return 0;

  return std::accumulate(test_suite->getTests().begin(), test_suite->getTests().end(), 0, [](int result, CppUnit::Test* test) {
      return result + get_case_count(test);
    });
}

void
progress_listener::startTest(CppUnit::Test *test) {
  std::cout << get_test_path(m_test_path) << test->getName() << std::flush;

  torrent::log_cleanup();

  m_last_test_failed = false;
  m_current_log_buffer = torrent::log_open_log_buffer("test_output");
}

void
progress_listener::addFailure(const CppUnit::TestFailure &failure) {
  // AddFailure is called for parent test suits, so only deal with leafs.
  if (m_current_log_buffer == nullptr)
    return;

  std::cout << " : " << (failure.isError() ? "error" : "assertion") << std::flush;

  m_last_test_failed = true;
  m_failures.push_back(failure_type{ failure.failedTestName(), std::move(m_current_log_buffer) });
}

void
progress_listener::endTest(CppUnit::Test *test) {
  std::cout << (m_last_test_failed ? "" : " : OK") << std::endl;

  m_current_log_buffer.reset();
  torrent::log_cleanup();
}

void
progress_listener::startSuite(CppUnit::Test *suite) {
  m_test_path.push_back(suite);

  if (suite->countTestCases() == 0)
    return;

  if (get_suite_count(suite) == 0) {
    std::cout << std::endl;
    return;
  }

  if (get_sub_case_count(suite) == 0)
    return;

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << std::string(suite->getName().size() + 1, '=') << std::endl;
  std::cout << suite->getName() << ":" << std::endl;
  std::cout << std::string(suite->getName().size() + 1, '=') << std::endl;
}

void
progress_listener::endSuite(CppUnit::Test *suite) {
  m_test_path.pop_back();
}
