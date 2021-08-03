#import "config.h"

#import "test_http.h"

#import <memory>
#import <sstream>

#import "mocks/http.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_http, "torrent");

#define HTTP_SETUP()                                                    \
  int done_counter = 0;                                                 \
  int failed_counter = 0;                                               \
  bool http_destroyed = false;                                          \
  bool stream_destroyed = false;                                        \
                                                                        \
  auto http_getter = new mocks::http_getter();                          \
  auto http_stream = new StringStream(&stream_destroyed);               \
                                                                        \
  http_getter->set_destroyed_status(&http_destroyed);                   \
  http_getter->set_stream(http_stream);                                 \
  http_getter->signal_done().push_back(std::bind(&increment_value, &done_counter)); \
  http_getter->signal_failed().push_back(std::bind(&increment_value, &failed_counter));

class StringStream : public std::stringstream {
public:
  StringStream(bool *destroyed) : m_destroyed(destroyed) {}
  ~StringStream() { *m_destroyed = true; }
private:
  bool* m_destroyed;
};

static void increment_value(int* value) { (*value)++; }

void
test_http::test_basic() {
  mocks::http_getter::slot_factory() = std::bind(&mocks::create_http_getter);

  auto http_getter = mocks::http_getter::slot_factory()();
  auto http_stream = std::make_unique<std::stringstream>();

  http_getter->set_url("http://example.com");
  CPPUNIT_ASSERT(http_getter->url() == "http://example.com");

  CPPUNIT_ASSERT(http_getter->stream() == NULL);
  http_getter->set_stream(http_stream.get());
  CPPUNIT_ASSERT(http_getter->stream() == http_stream.get());

  CPPUNIT_ASSERT(http_getter->timeout() == 0);
  http_getter->set_timeout(666);
  CPPUNIT_ASSERT(http_getter->timeout() == 666);
}

void
test_http::test_done() {
  HTTP_SETUP();
  http_getter->start();

  CPPUNIT_ASSERT(http_getter->trigger_signal_done());

  // Check that we didn't delete...

  CPPUNIT_ASSERT(done_counter == 1 && failed_counter == 0);
}

void
test_http::test_failure() {
  HTTP_SETUP();
  http_getter->start();

  CPPUNIT_ASSERT(http_getter->trigger_signal_failed());

  // Check that we didn't delete...

  CPPUNIT_ASSERT(done_counter == 0 && failed_counter == 1);
}

void
test_http::test_delete_on_done() {
  HTTP_SETUP();
  http_getter->start();
  http_getter->set_delete_stream();

  CPPUNIT_ASSERT(!stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->trigger_signal_done());
  CPPUNIT_ASSERT(stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->stream() == NULL);

  stream_destroyed = false;
  http_stream = new StringStream(&stream_destroyed);
  http_getter->set_stream(http_stream);

  http_getter->start();
  http_getter->set_delete_self();

  CPPUNIT_ASSERT(!stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->trigger_signal_done());
  CPPUNIT_ASSERT(stream_destroyed);
  CPPUNIT_ASSERT(http_destroyed);
}

void
test_http::test_delete_on_failure() {
  HTTP_SETUP();
  http_getter->start();
  http_getter->set_delete_stream();

  CPPUNIT_ASSERT(!stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->trigger_signal_failed());
  CPPUNIT_ASSERT(stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->stream() == NULL);

  stream_destroyed = false;
  http_stream = new StringStream(&stream_destroyed);
  http_getter->set_stream(http_stream);

  http_getter->start();
  http_getter->set_delete_self();

  CPPUNIT_ASSERT(!stream_destroyed);
  CPPUNIT_ASSERT(!http_destroyed);
  CPPUNIT_ASSERT(http_getter->trigger_signal_failed());
  CPPUNIT_ASSERT(stream_destroyed);
  CPPUNIT_ASSERT(http_destroyed);
}
