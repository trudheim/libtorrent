#import "config.h"

#import "mock/http.h"

namespace mock {

http_getter::http_getter() :
  m_test_flags(0),
  m_destroyed_status(nullptr) {}

http_getter::~http_getter() {
  if (m_destroyed_status != nullptr)
    *m_destroyed_status = true;
}

void
http_getter::start() {
  m_test_flags |= test_flag_active;
}

void
http_getter::close() {
  m_test_flags &= ~test_flag_active;
}

bool
http_getter::trigger_signal_done() {
  if (!(m_test_flags & test_flag_active))
    return false;

  m_test_flags &= ~test_flag_active;
  trigger_done();

  return true;
}

bool
http_getter::trigger_signal_failed() {
  if (!(m_test_flags & test_flag_active))
    return false;

  m_test_flags &= ~test_flag_active;
  trigger_failed("mock triggered failed");

  return true;
}

}
