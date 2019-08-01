#include "helpers/test_fixture.h"

class test_bind_manager : public test_fixture {
  CPPUNIT_TEST_SUITE(test_bind_manager);

  CPPUNIT_TEST(basic);
  CPPUNIT_TEST(backlog);
  CPPUNIT_TEST(flags);

  CPPUNIT_TEST(add_bind);
  CPPUNIT_TEST(add_bind_error);
  CPPUNIT_TEST(add_bind_priority);
  CPPUNIT_TEST(add_bind_v4mapped);
  CPPUNIT_TEST(remove_bind);

  CPPUNIT_TEST(connect_socket);
  CPPUNIT_TEST(connect_socket_error);
  CPPUNIT_TEST(connect_socket_v4bound);
  CPPUNIT_TEST(connect_socket_v6bound);
  CPPUNIT_TEST(connect_socket_v4only);
  CPPUNIT_TEST(connect_socket_v6only);
  CPPUNIT_TEST(connect_socket_block_connect);

  CPPUNIT_TEST(listen_socket_randomize);
  CPPUNIT_TEST(listen_socket_sequential);

  CPPUNIT_TEST(listen_open_bind);
  CPPUNIT_TEST(listen_open_bind_error);

  CPPUNIT_TEST_SUITE_END();

public:
  void basic();
  void backlog();
  void flags();

  void add_bind();
  void add_bind_error();
  void add_bind_priority();
  void add_bind_v4mapped();
  void remove_bind();

  void connect_socket();
  void connect_socket_error();
  void connect_socket_v4bound();
  void connect_socket_v6bound();
  void connect_socket_v4only();
  void connect_socket_v6only();
  void connect_socket_block_connect();

  void listen_socket_randomize();
  void listen_socket_sequential();

  void listen_open_bind();
  void listen_open_bind_error();
};
