#import "config.h"

#import "test_tracker_http.h"

#import "torrent/http.h"
#import "torrent/tracker_list.h"
#import "tracker/tracker_http.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_tracker_http, "tracker");

namespace test {
namespace tracker_http {

class http_getter : public torrent::Http {
public:
  static const int flag_active = 0x1;

  http_getter() : m_test_flags(0) {}
  ~http_getter() override {}
  
  void start() override { m_test_flags |= flag_active; }
  void close() override { m_test_flags &= ~flag_active; }

  bool trigger_signal_done();
  bool trigger_signal_failed();

private:
  int m_test_flags;
};

bool
http_getter::trigger_signal_done() {
  // if (!(m_test_flags & flag_active))
  //   return false;

  // m_test_flags &= ~flag_active;
  // trigger_done();
  return true;
}

bool
http_getter::trigger_signal_failed() {
  // if (!(m_test_flags & flag_active))
  //   return false;

  // m_test_flags &= ~flag_active;
  // trigger_failed("We Fail.");
  return true;
}

http_getter* create_http_getter() { return new http_getter; }

uint32_t controller_receive_success(torrent::Tracker*, torrent::AddressList*) { return 0; }
void     controller_receive_failure(torrent::Tracker*, const std::string&) {}
void     controller_receive_scrape(torrent::Tracker*) {}
void     controller_receive_tracker_enabled(torrent::Tracker*) {}
void     controller_receive_tracker_disabled(torrent::Tracker*) {}

}
}

void
test_tracker_http::test_basic() {
  // TODO: Replace TrackerList parameter with slots, then write these tests.


  // torrent::Http::slot_factory() = std::bind(&test::tracker_http::create_http_getter);

  // std::unique_ptr<torrent::TrackerList> tracker_list(new torrent::TrackerList());

  // tracker_list->slot_success() = std::bind(&test::tracker_http::controller_receive_success, std::placeholders::_1, std::placeholders::_2);
  // tracker_list->slot_failure() = std::bind(&test::tracker_http::controller_receive_failure, std::placeholders::_1, std::placeholders::_2);
  // tracker_list->slot_scrape_success() = std::bind(&test::tracker_http::controller_receive_scrape, std::placeholders::_1);
  // tracker_list->slot_tracker_enabled()  = std::bind(&test::tracker_http::controller_receive_tracker_enabled, std::placeholders::_1);
  // tracker_list->slot_tracker_disabled() = std::bind(&test::tracker_http::controller_receive_tracker_disabled, std::placeholders::_1);

  // tracker_list->insert_url(0, "http://example.com/announce", false);
}
