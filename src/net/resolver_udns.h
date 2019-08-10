#ifndef LIBTORRENT_NET_RESOLVER_UDNS_H
#define LIBTORRENT_NET_RESOLVER_UDNS_H

#include <cinttypes>
#include <functional>
#include <list>
#include <memory>

#include "torrent/event.h"
#include "torrent/connection_manager.h"
#include "rak/priority_queue_default.h"

struct dns_ctx;
struct dns_query;

namespace torrent {

struct query_udns {
  ::dns_query *a4_query;
  ::dns_query *a6_query;
  resolver_callback *callback;
  int error;
};

class resolver_udns : public Event {
public:
  typedef std::unique_ptr<query_udns> query_ptr;
  typedef std::vector<query_ptr>      query_list_type;

  resolver_udns();
  ~resolver_udns();

  void        event_read() override;
  void        event_write() override;
  void        event_error() override;
  const char* type_name() const override;

  void*       enqueue_resolve(const char* name, int family, resolver_callback* callback);
  void        flush_resolves();
  void        cancel(void* query_v);

protected:
  void        process_timeouts();

  ::dns_ctx*         m_ctx;
  rak::priority_item m_task_timeout;
  query_list_type    m_malformed_queries;
};

}

#endif
