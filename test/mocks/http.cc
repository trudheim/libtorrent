#import "config.h"

#import "mocks/http.h"

namespace mocks {

http_getter::http_getter() :
  m_flags(0),
  m_destroyed_status(nullptr) {}

http_getter::~http_getter() {
  if (m_destroyed_status != nullptr)
    *m_destroyed_status = true;
}

void
http_getter::start() {
  m_flags |= flag_active;
}

void
http_getter::close() {
  m_flags &= ~flag_active;
}

bool
http_getter::trigger_signal_done() {
  if (!(m_flags & flag_active))
    return false;

  m_flags &= ~flag_active;
  trigger_done();

  return true;
}

bool
http_getter::trigger_signal_failed() {
  if (!(m_flags & flag_active))
    return false;

  m_flags &= ~flag_active;
  trigger_failed("We Fail.");

  return true;
}

}
