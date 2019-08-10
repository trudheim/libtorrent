#include "config.h"

#ifdef USE_UDNS

#include "resolver_udns.h"

#include <netdb.h>
#include <udns.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "torrent/common.h"
#include "globals.h"
#include "manager.h"
#include "torrent/poll.h"

namespace torrent {

static int
udnserror_to_gaierror(int udnserror) {
  switch (udnserror) {
    case DNS_E_TEMPFAIL:
      return EAI_AGAIN;
    case DNS_E_PROTOCOL:
      // this isn't quite right
      return EAI_FAIL;
    case DNS_E_NXDOMAIN:
      return EAI_NONAME;
    case DNS_E_NODATA:
      return EAI_ADDRFAMILY;
    case DNS_E_NOMEM:
      return EAI_MEMORY;
    case DNS_E_BADQUERY:
      return EAI_NONAME;
    default:
      return EAI_ADDRFAMILY;
  }
}

// Compatibility layers so udns can call std::function callbacks.
static void
a4_callback_wrapper(struct ::dns_ctx *ctx, ::dns_rr_a4 *result, void *data) {
  struct sockaddr_in sa;

  // Udns will free the a4_query after this callback exits.
  query_udns *query = static_cast<query_udns*>(data);
  query->a4_query = nullptr;

  if (result == nullptr || result->dnsa4_nrr == 0) {
    if (query->a6_query == nullptr) {
      // nothing more to do: call the callback with a failure status
      (*(query->callback))(nullptr, udnserror_to_gaierror(::dns_status(ctx)));
      delete query;
    }
    // else: return and wait to see if we get an a6 response
  } else {
    sa.sin_family = AF_INET;
    sa.sin_port = 0;
    sa.sin_addr = result->dnsa4_addr[0];
    if (query->a6_query != nullptr) {
      ::dns_cancel(ctx, query->a6_query);
    }
    (*query->callback)(reinterpret_cast<sockaddr*>(&sa), 0);
    delete query;
  }
}

static void
a6_callback_wrapper(struct ::dns_ctx *ctx, ::dns_rr_a6 *result, void *data) {
  struct sockaddr_in6 sa;

  // Udns will free the a6_query after this callback exits.
  query_udns *query = static_cast<query_udns*>(data);
  query->a6_query = nullptr;

  if (result == nullptr || result->dnsa6_nrr == 0) {
    if (query->a4_query == nullptr) {
      // nothing more to do: call the callback with a failure status
      (*(query->callback))(nullptr, udnserror_to_gaierror(::dns_status(ctx)));
      delete query;
    }
    // else: return and wait to see if we get an a4 response
  } else {
    sa.sin6_family = AF_INET6;
    sa.sin6_port = 0;
    sa.sin6_addr = result->dnsa6_addr[0];
    if (query->a4_query != nullptr) {
      ::dns_cancel(ctx, query->a4_query);
    }
    (*query->callback)(reinterpret_cast<sockaddr*>(&sa), 0);
    delete query;
  }
}

resolver_udns::resolver_udns() {
  // reinitialize the default context, no-op
  // TODO don't do this here --- do it once in the manager, or in rtorrent
  ::dns_init(nullptr, 0);
  // thread-safe context isolated to this object:
  m_ctx = ::dns_new(nullptr);
  m_fileDesc = ::dns_open(m_ctx);

  if (m_fileDesc == -1)
    throw internal_error("dns_init failed");

  m_task_timeout.slot() = std::bind(&resolver_udns::process_timeouts, this);
}

resolver_udns::~resolver_udns() {
  priority_queue_erase(&taskScheduler, &m_task_timeout);
  ::dns_close(m_ctx);
  ::dns_free(m_ctx);
  m_fileDesc = -1;
  m_ctx = nullptr;
}

void
resolver_udns::event_read() {
  ::dns_ioevent(m_ctx, 0);
}

void
resolver_udns::event_write() {
}

void
resolver_udns::event_error() {
}

const char*
resolver_udns::type_name() const {
  return "resolver_udns";
}

// Wraps udns's dns_submit_a[46] functions. they and it return control
// immediately, without either sending outgoing UDP packets or
// executing callbacks.
void*
resolver_udns::enqueue_resolve(const char* name, int family, resolver_callback* callback) {
  query_ptr query(new query_udns { nullptr, nullptr, callback, 0 });

  if (family == AF_INET || family == AF_UNSPEC) {
    query->a4_query = ::dns_submit_a4(m_ctx, name, 0, a4_callback_wrapper, query.get());

    if (query->a4_query == nullptr) {
      // XXX udns does query parsing up front and will fail immediately
      // during submission of malformed domain names, e.g., `..`. In order to
      // maintain a clean interface, keep track of this query internally
      // so we can call the callback later with a failure code
      if (::dns_status(m_ctx) != DNS_E_BADQUERY)
        throw internal_error("resolver_udns call to dns_submit_a4 failed with unrecoverable error");

      // this is what getaddrinfo(3) would return:
      query->error = EAI_NONAME;

      m_malformed_queries.push_back(std::move(query));
      return m_malformed_queries.back().get();
    }
  }

  if (family == AF_INET6 || family == AF_UNSPEC) {
    query->a6_query = ::dns_submit_a6(m_ctx, name, 0, a6_callback_wrapper, query.get());

    if (query->a6_query == nullptr) {
      // it should be impossible for dns_submit_a6 to fail if dns_submit_a4
      // succeeded, but just in case, make it a hard failure:
      if (::dns_status(m_ctx) != DNS_E_BADQUERY || query->a4_query != nullptr)
        throw internal_error("resolver_udns call to dns_submit_a6 failed with unrecoverable error");

      query->error = EAI_NONAME;
      m_malformed_queries.push_back(std::move(query));
      return m_malformed_queries.back().get();
    }
  }

  // TODO: We should keep track of successfully started queries, not just malformed.
  return query.release();
}

// Wraps the dns_timeouts function. it sends packets and can execute
// arbitrary callbacks.
void
resolver_udns::flush_resolves() {
  while (!m_malformed_queries.empty()) {
    query_ptr query = std::move(m_malformed_queries.back());
    m_malformed_queries.pop_back();
    (*(query->callback))(nullptr, query->error);
  }

  process_timeouts();
}

// TODO: Make adding queries automatically insert_read/etc. Do not flush when adding.

void
resolver_udns::cancel(void* query_v) {
  auto query = static_cast<query_udns*>(query_v);

  if (query == nullptr)
    return;

  // TODO: Verify found in list first.

  if (query->a4_query != nullptr)
    ::dns_cancel(m_ctx, query->a4_query);

  if (query->a6_query != nullptr)
    ::dns_cancel(m_ctx, query->a6_query);

  auto itr = std::find_if(m_malformed_queries.begin(), m_malformed_queries.end(), [query](query_ptr& q){ return q.get() == query; });

  if (itr != m_malformed_queries.end())
    m_malformed_queries.erase(itr);
}

void
resolver_udns::process_timeouts() {
  int timeout = ::dns_timeouts(m_ctx, -1, 0);

  if (timeout == -1) {
    // no pending queries
    manager->poll()->remove_read(this);
    manager->poll()->remove_error(this);
  } else {
    manager->poll()->insert_read(this);
    manager->poll()->insert_error(this);
    priority_queue_erase(&taskScheduler, &m_task_timeout);
    priority_queue_insert(&taskScheduler, &m_task_timeout, (cachedTime + rak::timer::from_seconds(timeout)).round_seconds());
  }
}

}

#endif
