#include "config.h"

#include "test_resolver_udns.h"

#include "helpers/expect_fd.h"
#include "helpers/expect_utils.h"
#include "helpers/mock_function.h"
#include "helpers/network.h"
#include "net/resolver_udns.h"
#include "torrent/connection_manager.h"
#include "torrent/utils/log.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_resolver_udns, "net");

#define TEST_RESOLVER_BEGIN(name, nodename, family)                     \
  torrent::resolver_udns resolver;                                      \
  torrent::resolver_callback callback = std::bind(&mock_resolver_callback, std::placeholders::_1, std::placeholders::_2); \
  void* qptr = resolver.enqueue_resolve(nodename, family, &callback);

#define TEST_RESOLVER_ASSERT_INIT(query, resolve_nodename, resolve_family) \
  CPPUNIT_ASSERT(query.get() == qptr);                                  \
  CPPUNIT_ASSERT(query->hostname == std::string(resolve_nodename));     \
  CPPUNIT_ASSERT(query->family == resolve_family);                      \
  CPPUNIT_ASSERT(query->error == 0);                                    \
  CPPUNIT_ASSERT(query->resolver == &resolver);                         \
  CPPUNIT_ASSERT(query->callback == &callback);                         \
  CPPUNIT_ASSERT(query->a4_result == nullptr);                          \
  CPPUNIT_ASSERT(query->a6_result == nullptr);

namespace {

void
mock_resolver_callback(const sockaddr* sa, int err) {
  lt_log_print(torrent::LOG_MOCK_CALLS, "resolver: sa:%s err:%i", torrent::sa_pretty_str(sa).c_str(), err);
}

}

void
test_resolver_udns::basic() {
  torrent::resolver_udns resolver;

  // TODO: Use '"foo"s' when you upgrade to c++14.
  CPPUNIT_ASSERT(resolver.type_name() == std::string("resolver_udns"));
  CPPUNIT_ASSERT(resolver.is_open());
}

void
test_resolver_udns::enqueue_resolve() {
  { TEST_RESOLVER_BEGIN("localhost", "localhost", AF_UNSPEC);
    CPPUNIT_ASSERT(resolver.queries().size() == 1);
    CPPUNIT_ASSERT(resolver.queries().front()->a4_query != nullptr);
    CPPUNIT_ASSERT(resolver.queries().front()->a6_query != nullptr);
    TEST_RESOLVER_ASSERT_INIT(resolver.queries().front(), "localhost", AF_UNSPEC);
  };
  { TEST_RESOLVER_BEGIN("localhost", "localhost", AF_INET);
    CPPUNIT_ASSERT(resolver.queries().size() == 1);
    CPPUNIT_ASSERT(resolver.queries().front()->a4_query != nullptr);
    CPPUNIT_ASSERT(resolver.queries().front()->a6_query == nullptr);
    TEST_RESOLVER_ASSERT_INIT(resolver.queries().front(), "localhost", AF_INET);
  };
  { TEST_RESOLVER_BEGIN("localhost", "localhost", AF_INET6);
    CPPUNIT_ASSERT(resolver.queries().size() == 1);
    CPPUNIT_ASSERT(resolver.queries().front()->a4_query == nullptr);
    CPPUNIT_ASSERT(resolver.queries().front()->a6_query != nullptr);
    TEST_RESOLVER_ASSERT_INIT(resolver.queries().front(), "localhost", AF_INET6);
  };
}

void
test_resolver_udns::enqueue_resolve_error() {
  { torrent::resolver_udns resolver;
    CPPUNIT_ASSERT_THROW(resolver.enqueue_resolve("localhost", AF_UNSPEC, nullptr), torrent::internal_error);
  };
}
