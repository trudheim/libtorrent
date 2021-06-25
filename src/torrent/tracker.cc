#import "config.h"

#import <algorithm>

#import "exceptions.h"
#import "globals.h"
#import "tracker.h"
#import "tracker_list.h"

namespace torrent {

Tracker::Tracker(DownloadInfo* info, const std::string& url, int flags) :
  m_flags(flags),
  m_info(info),

  m_group(0),
  m_url(url),

  m_normal_interval(1800),
  m_min_interval(600),

  m_latest_event(EVENT_NONE),
  m_latest_new_peers(0),
  m_latest_sum_peers(0),

  m_success_time_last(0),
  m_success_counter(0),

  m_failed_time_last(0),
  m_failed_counter(0),

  m_scrape_time_last(0),
  m_scrape_counter(0),

  m_scrape_complete(0),
  m_scrape_incomplete(0),
  m_scrape_downloaded(0),

  m_request_time_last(torrent::cachedTime.seconds()),
  m_request_counter(0)
{
}

void
Tracker::enable() {
  if (is_enabled())
    return;

  m_flags |= flag_enabled;
  m_slot_tracker_enabled();
}

void
Tracker::disable() {
  if (!is_enabled())
    return;

  close();
  m_flags &= ~flag_enabled;
  m_slot_tracker_disabled();
}

uint32_t
Tracker::success_time_next() const {
  if (m_success_counter == 0)
    return 0;

  return m_success_time_last + m_normal_interval;
}

uint32_t
Tracker::failed_time_next() const {
  if (m_failed_counter == 0)
    return 0;

  return m_failed_time_last + (5 << std::min(m_failed_counter - 1, (uint32_t)6));
}

std::string
Tracker::scrape_url_from(std::string url) {
  size_t delim_slash = url.rfind('/');

  if (delim_slash == std::string::npos || url.find("/announce", delim_slash) != delim_slash)
    throw internal_error("Tried to make scrape url from invalid url.");

  return url.replace(delim_slash, sizeof("/announce") - 1, "/scrape");
}

void
Tracker::send_scrape() {
  throw internal_error("Tracker type does not support scrape.");
}

void
Tracker::inc_request_counter() {
  m_request_counter -= std::min(m_request_counter, (uint32_t)cachedTime.seconds() - m_request_time_last);
  m_request_counter++;
  m_request_time_last = cachedTime.seconds();

  if (m_request_counter >= 10)
    throw internal_error("Tracker request had more than 10 requests in 10 seconds.");
}

void
Tracker::clear_stats() {
  m_latest_new_peers = 0;
  m_latest_sum_peers = 0;

  m_success_counter = 0;
  m_failed_counter = 0;
  m_scrape_counter = 0;
}

}
