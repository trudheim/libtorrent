#ifndef LIBTORRENT_TRACKER_LIST_H
#define LIBTORRENT_TRACKER_LIST_H

#import <algorithm>
#import <functional>
#import <memory>
#import <string>
#import <vector>
#import <torrent/common.h>

namespace torrent {

class AddressList;
class DownloadInfo;
class DownloadWrapper;
class Tracker;

// The tracker list will contain a list of tracker, divided into
// subgroups. Each group must be randomized before we start. When
// starting the tracker request, always start from the beginning and
// iterate if the request failed. Upon request success move the
// tracker to the beginning of the subgroup and start from the
// beginning of the whole list.

class LIBTORRENT_EXPORT TrackerList : private std::vector<std::shared_ptr<Tracker>> {
public:
  friend class DownloadWrapper;

  typedef std::vector<std::shared_ptr<Tracker>> base_type;
  typedef AddressList                           address_list;

  using base_type::value_type;

  using base_type::iterator;
  using base_type::const_iterator;
  using base_type::reverse_iterator;
  using base_type::const_reverse_iterator;
  using base_type::size;
  using base_type::empty;

  using base_type::begin;
  using base_type::end;
  using base_type::rbegin;
  using base_type::rend;
  using base_type::front;
  using base_type::back;

  using base_type::at;
  using base_type::clear;
  using base_type::operator[];

  TrackerList(DownloadInfo* info);

  bool                has_active() const;
  bool                has_active_not_scrape() const;
  bool                has_active_in_group(uint32_t group) const;
  bool                has_active_not_scrape_in_group(uint32_t group) const;
  bool                has_usable() const;

  unsigned int        count_active() const;
  unsigned int        count_usable() const;

  void                close_all() { close_all_excluding(0); }
  void                close_all_excluding(int event_bitmap);

  void                disown_all_including(int event_bitmap);

  void                clear_stats();

  iterator            insert(unsigned int group, Tracker* tracker);
  void                insert_url(unsigned int group, const std::string& url, bool extra_tracker = false);

  void                send_state(Tracker* tracker, int new_event);
  void                send_state_idx(unsigned idx, int new_event);
  void                send_state_itr(iterator itr, int new_event);

  void                send_scrape(Tracker* tracker);

  DownloadInfo*       info()                                  { return m_info; }
  int                 state()                                 { return m_state; }

  uint32_t            key() const                             { return m_key; }
  void                set_key(uint32_t key)                   { m_key = key; }

  int32_t             numwant() const                         { return m_numwant; }
  void                set_numwant(int32_t n)                  { m_numwant = n; }

  auto find(Tracker* tb) -> iterator;

  // iterator            find(Tracker* tb)                       { return std::find(begin(), end(), tb); }
  iterator            find_url(const std::string& url);

  iterator            find_usable(iterator itr);
  const_iterator      find_usable(const_iterator itr) const;

  iterator            find_next_to_request(iterator itr);

  iterator            begin_group(unsigned int group);
  const_iterator      begin_group(unsigned int group) const;
  iterator            end_group(unsigned int group)           { return begin_group(group + 1); }
  const_iterator      end_group(unsigned int group) const     { return begin_group(group + 1); }

  size_type           size_group() const;
  void                cycle_group(unsigned int group);

  iterator            promote(iterator itr);
  void                randomize_group_entries();

  void                receive_success(Tracker* tb, AddressList* l);
  void                receive_failed(Tracker* tb, const std::string& msg);
  void                receive_scrape_success(Tracker* tb);
  void                receive_scrape_failed(Tracker* tb, const std::string& msg);
  void                receive_tracker_enabled(Tracker* t);
  void                receive_tracker_disabled(Tracker* t);

protected:
  void                set_state(int s)                        { m_state = s; }

private:
  TrackerList(const TrackerList&) = delete;
  void operator = (const TrackerList&) = delete;

  DownloadInfo*       m_info;
  int                 m_state;

  uint32_t            m_key;
  int32_t             m_numwant;

public:
  auto& slot_success()          { return m_slot_success; }
  auto& slot_failure()          { return m_slot_failed; }
  auto& slot_scrape_success()   { return m_slot_scrape_success; }
  auto& slot_scrape_failure()   { return m_slot_scrape_failed; }
  auto& slot_tracker_enabled()  { return m_slot_tracker_enabled; }
  auto& slot_tracker_disabled() { return m_slot_tracker_disabled; }

private:
  std::function<uint32_t (Tracker*, AddressList*)>   m_slot_success;
  std::function<void (Tracker*, const std::string&)> m_slot_failed;
  std::function<void (Tracker*)>                     m_slot_scrape_success;
  std::function<void (Tracker*, const std::string&)> m_slot_scrape_failed;
  std::function<void (Tracker*)>                     m_slot_tracker_enabled;
  std::function<void (Tracker*)>                     m_slot_tracker_disabled;
};

inline void
TrackerList::send_state_idx(unsigned idx, int new_event) {
  send_state(at(idx).get(), new_event);
}

inline void
TrackerList::send_state_itr(iterator itr, int new_event) {
  if (itr == end())
    return;
    
  send_state(itr->get(), new_event);
}

inline TrackerList::iterator
TrackerList::find(Tracker* tb) {
  return std::find_if(begin(), end(), [tb](auto& v){ return v.get() == tb; });
}

}

#endif
