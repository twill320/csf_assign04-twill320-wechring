#include <sstream>
#include <cctype>
#include <cassert>
#include <bits/stdc++.h>
#include <cstring>
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
  char usable_hostname[hostname.length()+1];
  strcpy(usable_hostname, hostname.c_str());

  std::string str = std::to_string(port);
  const char* char_port = str.c_str();
  m_fd = open_clientfd(usable_hostname, char_port);
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if (m_fd > 0) {
    ::close(m_fd);
  }
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
    ::close(m_fd);
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  std::string msg_str = msg.tag + ':' + msg.data + '\n';

  // msg length greater than max length
  if (msg_str.size() > msg.MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  // write message to server
  ssize_t nw = rio_writen(m_fd, &msg_str[0], msg_str.size());
  if (nw == -1) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  m_last_result = SUCCESS;

  return true;
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  // read message from server
  std::string tag;
  std::string data;
  //rio_readinitb(&m_fdbuf, m_fd);
  char buf[Message::MAX_LEN + 2];
  ssize_t n = rio_readlineb(&m_fdbuf, buf, sizeof(buf));

  // msg length greater than max length
  if (n > Message::MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  size_t i = 0;
  while (i < sizeof(buf) && buf[i] != ':') {
    tag += buf[i];
    i++;
  }
  i++;

  while (i < sizeof(buf) && buf[i] != '\r' && buf[i] != '\n') {
    data += buf[i];
    i++;
  }

  msg.tag = tag;
  msg.data = data;

  // check that message was received
  if (n > 0) {
    m_last_result = SUCCESS;
    return true;
  } else {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

}
