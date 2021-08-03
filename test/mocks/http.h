#import "torrent/http.h"

namespace mocks {

// TODO: Refactor test_tracker_list.
// TODO: Refacotr http stream to use shared_ptr.

class http_getter : public torrent::Http {
public:
  static const int flag_active = 0x1;

  http_getter();
  ~http_getter() override;

  void start() override { m_test_flags |= flag_active; }
  void close() override { m_test_flags &= ~flag_active; }

  bool trigger_signal_done();
  bool trigger_signal_failed();

  void set_destroyed_status(bool* status) { m_destroyed_status = status; }

private:
  int   m_test_flags;
  bool* m_destroyed_status;
};

inline http_getter* create_http_getter() { return new http_getter; }

inline http_getter::http_getter() :
  m_test_flags(0),
  m_destroyed_status(nullptr) {}

inline http_getter::~http_getter() {
  if (m_destroyed_status != nullptr)
    *m_destroyed_status = true;
}

// TODO: Move.
// uint32_t controller_receive_success(torrent::Tracker*, torrent::AddressList*) { return 0; }
// void     controller_receive_failure(torrent::Tracker*, const std::string&) {}
// void     controller_receive_scrape(torrent::Tracker*) {}
// void     controller_receive_tracker_enabled(torrent::Tracker*) {}
// void     controller_receive_tracker_disabled(torrent::Tracker*) {}

}
