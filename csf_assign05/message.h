#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>

struct Message {
  // An encoded message may have at most this many characters,
  // including the trailing newline ('\n'). Note that this does
  // *not* include a NUL terminator (if one is needed to
  // temporarily store the encoded message.)
  static const unsigned MAX_LEN = 255;

  std::string tag;
  std::string data;

  Message() { }

  Message(const std::string &tag, const std::string &data)
    : tag(tag), data(data) { }

  // TODO: you could add helper functions

  // to be used in send in connection.cpp
  std::string* make_msg_str() const {
    std::string* msg_str;
    *msg_str = tag + ":" + data + "\n";
    return msg_str;
  }

  // to be used in receive in connection.cpp
  void get_msg_comps(std::string buf) {
    size_t delim_ind = buf.find(':');

    if (!buf.empty() && buf.back() == '\n') buf.pop_back();
    if (!buf.empty() && buf.back() == '\r') buf.pop_back();

    tag = buf.substr(0, delim_ind);
    data = buf.substr(delim_ind + 1);
  }
};

// standard message tags (note that you don't need to worry about
// "senduser" or "empty" messages)
#define TAG_ERR       "err"       // protocol error
#define TAG_OK        "ok"        // success response
#define TAG_SLOGIN    "slogin"    // register as specific user for sending
#define TAG_RLOGIN    "rlogin"    // register as specific user for receiving
#define TAG_JOIN      "join"      // join a chat room
#define TAG_LEAVE     "leave"     // leave a chat room
#define TAG_SENDALL   "sendall"   // send message to all users in chat room
#define TAG_SENDUSER  "senduser"  // send message to specific user in chat room
#define TAG_QUIT      "quit"      // quit
#define TAG_DELIVERY  "delivery"  // message delivered by server to receiving client
#define TAG_EMPTY     "empty"     // sent by server to receiving client to indicate no msgs available

#endif // MESSAGE_H
