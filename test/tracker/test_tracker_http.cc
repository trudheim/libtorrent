#import "config.h"

#import "test_tracker_http.h"

#import "mock/http.h"
#import "torrent/tracker_list.h"
#import "tracker/tracker_http.h"

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_tracker_http, "tracker");

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
