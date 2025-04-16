#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);

  // connection error handling
  if (!conn.is_open()) {
    std::cerr << "Error: failed connection" << std::endl;
    exit( 1 );
  }

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message msg(TAG_RLOGIN, username);
  bool msg_sent = conn.send(msg);
  if (!msg_sent) {
    std::cerr << "Error: Failed to send rlogin message." << std::endl;
    return 1;
  }

  bool msg_received = conn.receive(msg);
  if (!msg_received) {
    std::cerr << "Error: No response for rlogin." << std::endl;
    return 1;
  }

  if (msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    exit( 1 );
  }

  msg.tag = TAG_JOIN;
  msg.data = room_name;
  msg_sent = conn.send(msg);
  if (!msg_sent) {
    std::cerr << "Error: Failed to send join message." << std::endl;
    return 1;
  }

  msg_received = conn.receive(msg);
  if (!msg_received) {
    std::cerr << "Error: No response for join." << std::endl;
    return 1;
  }

  if (!msg.tag.empty() && msg.tag.back() == '\r') {
    msg.tag.pop_back();
  }
  if (msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    exit( 1 );
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)

  while (conn.receive(msg)) {
    if (!msg.data.empty() && msg.data.back() == '\r') {
      msg.data.pop_back();
    }
    if (msg.tag == TAG_DELIVERY) {
      size_t pos1 = msg.data.find(':');
      if (pos1 == std::string::npos) { continue; } // Malformed message.
      size_t pos2 = msg.data.find(':', pos1 + 1);
      if (pos2 == std::string::npos) { continue; } // Malformed message.

      std::string sender = msg.data.substr(pos1 + 1, pos2 - pos1 - 1);
      std::string message = msg.data.substr(pos2 + 1);
      std::cout << sender << ": " << message << std::endl;
    }
    else if (msg.tag == "err") {
      std::string err = msg.data;
      if (!err.empty() && err.back() == '\r') err.pop_back();
      std::cerr << msg.data << std::endl;
    }
  }

  return 0;
}
