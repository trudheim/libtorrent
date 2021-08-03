#ifndef LIBTORRENT_HTTP_H
#define LIBTORRENT_HTTP_H

#include <string>
#include <functional>
#include <iosfwd>
#include <list>
#include <torrent/common.h>

namespace torrent {

// The client should set the user agent to something like
// "CLIENT_NAME/CLIENT_VERSION/LIBRARY_VERSION".

// Keep in mind that these objects get reused.
class LIBTORRENT_EXPORT Http {
 public:
  typedef std::function<void ()>                   slot_void;
  typedef std::function<void (const std::string&)> slot_string;
  typedef std::function<Http* (void)>              slot_http;

  typedef std::list<slot_void>   signal_void;
  typedef std::list<slot_string> signal_string;

  static const int flag_delete_self   = 0x1;
  static const int flag_delete_stream = 0x2;

  Http() : m_flags(0), m_stream(NULL), m_timeout(0) {}
  virtual ~Http();

  // Start must never throw on bad input. Calling start/stop on an
  // object in the wrong state should throw a torrent::internal_error.
  virtual void start() = 0;
  virtual void close() = 0;

  auto flags() const -> int;

  void set_delete_self();
  void set_delete_stream();

  auto url() const -> const std::string&;
  void set_url(const std::string& url);

  // Make sure the output stream does not have any bad/failed bits set.
  auto stream() -> std::iostream*;
  void set_stream(std::iostream* str);

  auto timeout() const -> uint32_t;
  void set_timeout(uint32_t seconds);

  // The owner of the Http object must close it as soon as possible
  // after receiving the signal, as the implementation may allocate
  // limited resources during its lifetime.
  auto signal_done() -> signal_void&;
  auto signal_failed() -> signal_string&;

  // Guaranteed to return a valid object or throw a internal_error. The
  // caller takes ownership of the returned object.
  static slot_http&  slot_factory()                       { return m_factory; }

protected:
  void trigger_done();
  void trigger_failed(const std::string& message);

  int            m_flags;
  std::string    m_url;
  std::iostream* m_stream;
  uint32_t       m_timeout;

  signal_void    m_signal_done;
  signal_string  m_signal_failed;

private:
  Http(const Http&);
  void operator = (const Http&);

  static slot_http m_factory;
};

inline auto  Http::flags() const -> int { return m_flags; }
inline void  Http::set_delete_self() { m_flags |= flag_delete_self; }
inline void  Http::set_delete_stream() { m_flags |= flag_delete_stream; }
inline auto  Http::url() const -> const std::string& { return m_url; }
inline void  Http::set_url(const std::string& url) { m_url = url; }
inline auto  Http::stream() -> std::iostream* { return m_stream; }
inline void  Http::set_stream(std::iostream* str) { m_stream = str; }
inline auto  Http::timeout() const -> uint32_t { return m_timeout; }
inline void  Http::set_timeout(uint32_t seconds) { m_timeout = seconds; }
inline auto  Http::signal_done() -> signal_void& { return m_signal_done; }
inline auto  Http::signal_failed() -> signal_string& { return m_signal_failed; }

}

#endif
