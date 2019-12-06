#include "config.h"

#define __STDC_FORMAT_MACROS

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <rak/socket_address.h>
#include <sys/socket.h>

#include "torrent/exceptions.h"
#include "torrent/connection_manager.h"
#include "torrent/poll.h"
#include "torrent/net/bind_manager.h"
#include "torrent/net/socket_address.h"
#include "torrent/utils/log.h"

#include "listen.h"
#include "manager.h"

#define LT_LOG(log_fmt, ...)                                            \
  lt_log_print(LOG_CONNECTION_LISTEN, "listen: " log_fmt, __VA_ARGS__);
#define LT_LOG_SOCKADDR(log_fmt, sa, ...)                               \
  lt_log_print(LOG_CONNECTION_LISTEN, "listen->%s: " log_fmt, sa_pretty_str(sa).c_str(), __VA_ARGS__);

namespace torrent {

bool
Listen::open() {
  close();

  listen_result_type listen_result = manager->bind()->listen_socket(0);
  m_fileDesc = listen_result.fd;
  m_sockaddr.swap(listen_result.address);

  if (m_fileDesc == -1) {
    LT_LOG("failed to open listen port", 0);
    return false;
  }

  manager->connection_manager()->inc_socket_count();

  manager->poll()->open(this);
  manager->poll()->insert_read(this);
  manager->poll()->insert_error(this);

  LT_LOG_SOCKADDR("listen port %" PRIu16 " opened", m_sockaddr.get(), sa_port(m_sockaddr.get()));
  return true;
}

void Listen::close() {
  if (!get_fd().is_valid())
    return;

  manager->poll()->remove_read(this);
  manager->poll()->remove_error(this);
  manager->poll()->close(this);

  manager->connection_manager()->dec_socket_count();

  get_fd().close();
  get_fd().clear();
  
  m_sockaddr.release();
}
  
uint16_t
Listen::port() const {
  return m_sockaddr ? sa_port(m_sockaddr.get()) : 0;
}

void
Listen::event_read() {
  rak::socket_address sa;
  SocketFd fd;

  while ((fd = get_fd().accept(&sa)).is_valid()) {
    LT_LOG("accepted connection (fd:%i address:%s)", fd.get_fd(), sa_pretty_str(sa.c_sockaddr()).c_str());

    m_slot_accepted(fd.get_fd(), sa.c_sockaddr());
  }
}

void
Listen::event_write() {
  throw internal_error("Listener does not support write().");
}

void
Listen::event_error() {
  int error = get_fd().get_error();

  LT_LOG("event_error: %s", std::strerror(error));

  if (error != 0)
    throw internal_error("Listener port received an error event: " + std::string(std::strerror(error)));
}

}
