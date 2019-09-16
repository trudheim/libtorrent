#ifndef LIBTORRENT_NET_RESOLVER_UDNS_H
#define LIBTORRENT_NET_RESOLVER_UDNS_H

#include <cinttypes>
#include <functional>
#include <list>
#include <memory>

#include "torrent/net/socket_event.h"
#include "torrent/connection_manager.h"
#include "torrent/net/socket_address.h"

#include "rak/priority_queue_default.h"

struct dns_ctx;
struct dns_query;

namespace torrent {

class resolver_udns;

struct query_udns {
  std::string hostname;
  int family;
  int error;

  resolver_udns* resolver;
  resolver_callback *callback;
  ::dns_query* a4_query;
  ::dns_query* a6_query;

  sa_unique_ptr a4_result;
  sa_unique_ptr a6_result;
};

class resolver_udns : public socket_event {
public:
  typedef std::unique_ptr<query_udns> query_ptr;
  typedef std::vector<query_ptr>      query_list_type;

  resolver_udns();
  ~resolver_udns();

  void         event_read() override;
  const char*  type_name() const override;

  void*        enqueue_resolve(const char* hostname, int family, resolver_callback* callback);
  void         flush_resolves();
  void         cancel(void* v_query);

  static void  complete_query(query_udns* query);
  static auto  erase_query(query_udns* query) -> resolver_udns::query_ptr;

  // Get const query.

  auto         queries() const -> const query_list_type&;
  auto         completed_queries() const -> const query_list_type&;
  auto         malformed_queries() const -> const query_list_type&;

private:
  void         process_timeouts();
  bool         enqueue_numeric(const char* hostname, int family, query_ptr& query);

  ::dns_ctx*         m_ctx;
  rak::priority_item m_task_timeout;

  query_list_type    m_queries;
  query_list_type    m_completed_queries;
  query_list_type    m_malformed_queries;
};

inline const resolver_udns::query_list_type& resolver_udns::queries() const { return m_queries; }
inline const resolver_udns::query_list_type& resolver_udns::completed_queries() const { return m_completed_queries; }
inline const resolver_udns::query_list_type& resolver_udns::malformed_queries() const { return m_malformed_queries; }

}

#endif
