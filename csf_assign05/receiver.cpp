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

  // receiver login
  Message msg(TAG_RLOGIN, username);
  bool msg_sent = conn.send(msg);
  if (!msg_sent) { // check that login request sent
    std::cerr << "Error: Failed to send rlogin message." << std::endl;
    conn.close();
    return 1;
  }

  bool msg_received = conn.receive(msg);
  if (!msg_received) { // check that login response was received
    std::cerr << "Error: No response for rlogin." << std::endl;
    conn.close();
    return 1;
  }

  if (msg.tag == TAG_ERR) { // check for error resonse
    std::cerr << msg.data << std::endl;
    conn.close();
    exit( 1 );
  }

  msg.tag = TAG_JOIN;
  msg.data = room_name;
  msg_sent = conn.send(msg);
  if (!msg_sent) {
    std::cerr << "Error: Failed to send join message." << std::endl;
    conn.close();
    return 1;
  }

  Message join_response;
  msg_received = conn.receive(join_response);
  if (!msg_received) {
    std::cerr << "Error: No response for join." << std::endl;
    conn.close();
    return 1;
  }

  if (join_response.tag == TAG_ERR) { // check for error response
    std::cerr << join_response.data << std::endl;
    conn.close();
    exit( 1 );
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)

  while (conn.receive(msg)) {
    if (msg.tag == TAG_DELIVERY) {
      size_t i = 0;
      std::string room;
      std::string sender;
      std::string message;

      while (msg.data[i] != ':') {
        room += msg.data[i];
        i++;
      }
      i++;

      while (msg.data[i] != ':') {
        sender += msg.data[i];
        i++;
      }
      i++;

      while (i < msg.data.size()) {
        message += msg.data[i];
        i++;
      }

      std::cout << sender << ": " << message << std::endl;
    }
    else if (msg.tag == "err") {
      std::cerr << msg.data;
    }
  }

  return 0;
}
