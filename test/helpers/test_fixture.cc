#include "config.h"

#include "test_fixture.h"

#include "globals.h"
#include "torrent/utils/log.h"
#include "rak/timer.h"

void
test_fixture::setUp() {
  mock_clear_ignore_assert();

  CPPUNIT_ASSERT(torrent::taskScheduler.empty());

  // TODO: Start replacing cachedTime with clock_gettime(CLOCK_REALTIME_COARSE, ...).
  torrent::cachedTime = rak::timer::current();

  log_add_group_output(torrent::LOG_MOCK_CALLS, "test_output");
}

void
test_fixture::tearDown() {
  mock_clear();

  torrent::taskScheduler.clear();
}
