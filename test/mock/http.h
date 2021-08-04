#import "torrent/http.h"

namespace mock {

class http_getter : public torrent::Http {
public:
  static const int test_flag_active = 0x1;

  http_getter();
  ~http_getter() override;

  void start() override;
  void close() override;

  bool trigger_signal_done();
  bool trigger_signal_failed();

  void set_destroyed_status(bool* status);

private:
  int   m_test_flags;
  bool* m_destroyed_status;
};

inline http_getter* create_http_getter() { return new http_getter; }

inline void http_getter::set_destroyed_status(bool* status) { m_destroyed_status = status; }

}
