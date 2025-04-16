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
  char usable_hostname[hostname.length()];
  strcpy(usable_hostname, hostname.c_str());

  std::string str = std::to_string(port);
  const char* char_port = str.c_str();
  m_fd = open_clientfd(usable_hostname, char_port);
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
  unsigned msg_len = msg.data.length() + 1 + msg.tag.length();
  std::string* msg_str = msg.make_msg_str();

  //const char* char_str = temp_string.c_str();

  // Cast the const char* to const void*
  //const void* void_str = static_cast<const void*>(char_str);

  // msg length greater than max length
  if (msg_len > msg.MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  // write message to server
  ssize_t nw = rio_writen(m_fd, msg_str, msg_str->length());
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
  char buf[msg.MAX_LEN];
  rio_readinitb(&m_fdbuf, m_fd);
  ssize_t n = rio_readlineb(&m_fdbuf, buf, sizeof(buf));

  // trying to loop through buffer to get rid of newline
  if (buf[n - 1] == '\n') {
    buf[n - 1] = '\0';
  }

  // separate message into tag and data
  // buf already contains "<tag>:<data>\0"
  char *colon = strchr(buf, ':');        // find first ':' in the buffer

  *colon = '\0';                         // overwrite ':' with NUL → two C‑strings
  const char *tag_cstr  = buf;           // part before ':'
  const char *data_cstr = colon + 1;     // part after ':'

  // copy into std::string members
  msg.tag  = tag_cstr;
  msg.data = data_cstr;

  unsigned msg_len = msg.data.length() + 1 + msg.tag.length();

  // msg length greater than max length
  if (msg_len > msg.MAX_LEN) {
    m_last_result = INVALID_MSG;
    return false;
  }

  // check that message was received
  if (n > 0) {
    m_last_result = SUCCESS;
    return true;
  } else {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

}
