#import "torrent/http.h"

namespace mocks {

// TODO: Refactor test_tracker_list.

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

// TODO: Refactor http factory to use shared_ptr.
inline http_getter* create_http_getter() { return new http_getter; }

// TODO: Move.
// uint32_t controller_receive_success(torrent::Tracker*, torrent::AddressList*) { return 0; }
// void     controller_receive_failure(torrent::Tracker*, const std::string&) {}
// void     controller_receive_scrape(torrent::Tracker*) {}
// void     controller_receive_tracker_enabled(torrent::Tracker*) {}
// void     controller_receive_tracker_disabled(torrent::Tracker*) {}

}
