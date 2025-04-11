#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  // TODO: call rio_readinitb to initialize the rio_t object
  m_fd = open_clientfd(hostname, port);
  rio_readinitb(&m_fdbuf, port);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  Close(m_fd);
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  if (m_fd > 0) {
    return true;
  }
  return false;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (m_fd > 0) {
    Close(m_fd);
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  unsigned msg_len = strlen(msg->data);
  std::string *temp_string *msg->tag + *msg->data;

  // msg length greater than max length
  if (msg_len > msg->MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  rio_writen(m_fd, temp_string, strlen(temp_string));
  rio_writen(fd, "\n", 1);
  m_last_result = SUCCESS;

  return true
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  unsigned msg_len = strlen(msg->data);
  std::string *temp_string *msg->tag + *msg->data;

  // msg length greater than max length
  if (msg_len > msg->MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  char buf[msg->MAX_LEN];
  rio_readinitb(&m_fdbuf, m_fd);
  ssize_t n = rio_readlineb(&m_fdbuf, buf, sizeof(buf));

  vector<std::string> msg_line;
  getline(buf, msg_line, ': ');

  *msg->tag = msg_line[0];
  *msg->data = msg_line[1];

  if (n > 0) {
    m_last_result = SUCCESS;
    return true;
  } else {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

}
