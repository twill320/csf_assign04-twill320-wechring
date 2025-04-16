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
    fprintf( stderr, "Error: couldn't connect to server" );
    exit( 1 );
  }

  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message msg(TAG_RLOGIN, username);
  conn.send(msg);

  conn.receive(msg);
  if (msg.tag == TAG_ERR) {
    fprintf( stderr, msg.data.c_str() );
    msg.tag = TAG_QUIT;
    conn.send(msg);
    exit( 1 );
  }

  msg.tag = TAG_JOIN;
  msg.data = room_name;
  conn.send(msg);

  conn.receive(msg);
  if (msg.tag == TAG_ERR) {
    fprintf( stderr, msg.data.c_str() );
    msg.tag = TAG_QUIT;
    conn.send(msg);
    exit( 1 );
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  
  // still have to work on this part for reading delivered messages
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
