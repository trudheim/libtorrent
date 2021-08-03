#import "config.h"

#import "mocks/http.h"

namespace mocks {

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
