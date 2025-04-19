#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server
  Connection conn;
  conn.connect(server_hostname, server_port);

  if (!conn.is_open()) {
    std::cerr << "Error: failed connection" << std::endl;
    exit(1);
  }

  // TODO: send slogin message
  Message msg(TAG_SLOGIN, username);
  bool msg_sent = conn.send(msg);
  if (!msg_sent) {
    std::cerr << "Error: Failed to send slogin message." << std::endl;
    conn.close();
    return 1;
  }

  bool msg_received = conn.receive(msg);
  if (!msg_received) {
    std::cerr << "Error: No response for slogin." << std::endl;
    conn.close();
    return 1;
  }

  if (msg.tag == TAG_ERR) {
    std::cerr << msg.data << std::endl;
    conn.close();
    return 1;
  }

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  std::string input;
  while (std::getline(std::cin, input)) {
    if (input.empty()) {
      continue;
    }
    //Commands
    if (input[0] == '/') {
      std::string command, arg;
      size_t space_pos = input.find(' ');
      if (space_pos != std::string::npos) {
        command = input.substr(1, space_pos - 1);
        arg = input.substr(space_pos + 1);
      } else {
        command = input.substr(1);
      }

      if (command == "join") {
        msg = Message(TAG_JOIN, arg);
      } else if (command == "leave") {
        msg = Message(TAG_LEAVE, "");
      } else if (command == "quit") {
        msg = Message(TAG_QUIT, "");
        conn.send(msg); // Try to send quit before exiting
        break;
      } else {
        std::cerr << "Unknown command: " << command << std::endl;
        continue;
      }
    } else {
      // Messages
      msg = Message(TAG_SENDALL, input);
    }

    msg_sent = conn.send(msg);
    if (!msg_sent) {
      std::cerr << "Error: Failed to send message." << std::endl;
      continue;
    }

    // Check for server response
    msg_received = conn.receive(msg);
    if (!msg_received) {
      std::cerr << "Error: No response from server." << std::endl;
      break;
    }

    if (msg.tag == TAG_ERR) {
      std::cerr << msg.data << std::endl;
    }
  }

  conn.close();
  return 0;
}
