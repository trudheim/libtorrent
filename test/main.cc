#import "config.h"

#import <algorithm>
#import <cstdlib>
#import <iostream>
#import <stdexcept>
#import <signal.h>
#import <string.h>
#import <cppunit/BriefTestProgressListener.h>
#import <cppunit/CompilerOutputter.h>
#import <cppunit/TestResult.h>
#import <cppunit/TestResultCollector.h>
#import <cppunit/extensions/HelperMacros.h>
#import <cppunit/extensions/TestFactoryRegistry.h>
#import <cppunit/ui/text/TestRunner.h>

#ifdef HAVE_BACKTRACE
#import <execinfo.h>
#endif

#import "helpers/progress_listener.h"
#import "torrent/utils/log.h"

CPPUNIT_REGISTRY_ADD_TO_DEFAULT("torrent/net");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("torrent/utils");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("torrent");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("torrent::tracker_list");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("torrent::tracker_controller");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("net");
CPPUNIT_REGISTRY_ADD_TO_DEFAULT("tracker");

void
do_test_panic(int signum) {
  signal(signum, SIG_DFL);

  std::cout << std::endl << std::endl << "Caught " << strsignal(signum) << ", dumping stack:" << std::endl << std::endl;
  
#ifdef HAVE_BACKTRACE
  void* stackPtrs[20];

  // Print the stack and exit.
  int stackSize = backtrace(stackPtrs, 20);
  char** stackStrings = backtrace_symbols(stackPtrs, stackSize);

  for (int i = 0; i < stackSize; ++i)
    std::cout << stackStrings[i] << std::endl;

#else
  std::cout << "Stack dump not enabled." << std::endl;
#endif

  std::cout << std::endl;
  torrent::log_cleanup();
  std::abort();
}

void
register_signal_handlers() {
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  sa.sa_handler = &do_test_panic;

  if (sigaction(SIGSEGV, &sa, NULL) == -1) {
    std::cout << "Could not register signal handlers." << std::endl;
    exit(-1);
  }
}

static
void add_tests(CppUnit::TextUi::TestRunner& runner, const char* c_test_names) {
  if (c_test_names == NULL || std::string(c_test_names).empty()) {
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    return;
  }

  const std::string& test_names(c_test_names);

  size_t pos = 0;
  size_t next = 0;

  while ((next = test_names.find(',', pos)) < test_names.size()) {
    auto name = test_names.substr(pos, next - pos);
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry(name).makeTest());
    pos = next + 1;
  }

  auto name = test_names.substr(pos);
  runner.addTest(CppUnit::TestFactoryRegistry::getRegistry(name).makeTest());
}

static void
dump_failure_log(const failure_type& failure) {
  if (failure.log->empty())
    return;

  std::cout << std::endl << failure.name << std::endl;

  // Doesn't print dump messages as log_buffer drops them.
  std::for_each(failure.log->begin(), failure.log->end(), [](const torrent::log_entry& entry) {
      std::cout << entry.timestamp << ' ' << entry.message << '\n';
    });

  std::cout << std::flush;
}

static void
dump_failures(const failure_list_type& failures) {
  if (failures.empty())
    return;

  std::cout << std::endl
            << "=================" << std::endl
            << "Failed Test Logs:" << std::endl
            << "=================" << std::endl;

  std::for_each(failures.begin(), failures.end(), [](const failure_type& failure) {
      dump_failure_log(failure);
    });
  std::cout << std::endl;
}

int main(int argc, char* argv[]) {
  register_signal_handlers();

  CppUnit::TestResult controller;
  CppUnit::TestResultCollector result;
  progress_listener progress;

  controller.addListener( &result );        
  controller.addListener( &progress );

  CppUnit::TextUi::TestRunner runner;
  add_tests(runner, std::getenv("TEST_NAME"));

  try {
    std::cout << "Running ";
    runner.run( controller );
 
    // TODO: Make outputter.
    dump_failures(progress.failures());

    // Print test in a compiler compatible format.
    CppUnit::CompilerOutputter outputter( &result, std::cerr );
    outputter.write();                      

  } catch ( std::invalid_argument &e ) { // Test path not resolved
    std::cerr  <<  std::endl <<  "ERROR: "  <<  e.what() << std::endl;
    return 1;
  }

  return result.wasSuccessful() ? 0 : 1;
}
