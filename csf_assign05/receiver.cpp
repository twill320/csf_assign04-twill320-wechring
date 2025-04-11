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

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message msg("rlogin", username);

  conn.send(msg);
  conn.receive(msg);

  if (msg.tag == TAG_ERR) {
    fprintf( stderr, "Error: could not register as specific user for receiving" );
    msg.tag = "quit";
    exit( 1 );
  }

  msg.tag = "join";
  msg.data = room_name;
  conn.send(msg);
  conn.receive(msg);

  if (msg.tag == TAG_ERR) {
    fprintf( stderr, "Error: could not register as specific user for receiving" );
    msg.tag = "quit";
    exit( 1 );
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  std::vector<std::string> data_vector;
  std::string data;

  std::istringstream iss(msg.data);
  while (msg.tag == TAG_DELIVERY && getline(iss, data, ':')) { // trying to get room, sender, and messsage info from data
    data_vector.push_back(data);
    // probably save room, sender, message info here
    // work with info here to print information
    data_vector.clear();
    // wait here
    // maybe use helper function similar to quicksort_wait
    conn.receive(msg);
    std::istringstream iss(msg.data);
  }


  return 0;
}
