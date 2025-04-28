#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

struct ConnInfo {
  int client_fd;
  Server *server;
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void chat_with_receiver(Server &server, Connection &conn, const std::string username) {
  Message msg;
  User user(username);
  conn.receive(msg);
  if (msg.tag != TAG_JOIN) { // want to handle case where server does not receive join message from client
    msg.data = TAG_ERR;
    msg.data = "Error: Did not receive join request.";
    conn.send(msg);
  }
  Room *room = server.find_or_create_room(msg.data);
  room->add_member(&user);
  msg.tag = TAG_OK;
  msg.data = "Join successful";
  conn.send(msg);

  // retrieve new messsasges form server while connection is open and message queue is non-empty
  MessageQueue &mqueue = user.mqueue;
  while (conn.is_open()) {
    Message *new_msg = mqueue.dequeue();
    if (new_msg == nullptr) {
      break;
    }
    conn.send(*new_msg);
    new_msg = mqueue.dequeue();
  }
}

void chat_with_sender(Server &server, Connection &conn, const std::string username) {
  Message msg;
  User user(username);
  conn.receive(msg);
  if (msg.tag != TAG_JOIN) { // want to handle case where server does not receive join message from client
    msg.data = TAG_ERR;
    msg.data = "Error: Did not receive join request.";
    conn.send(msg);
  }
  Room *room = server.find_or_create_room(msg.data);
  room->add_member(&user);
  msg.tag = TAG_OK;
  msg.data = "Join successful";
  conn.send(msg);

  // need to compare return value to size of message transmitted
  while (conn.receive(msg)) {
    if (msg.tag == "sendall") {
      room->broadcast_message(username, msg.data); // may need to use broadcast_message to update message queue for all other users
      conn.send(Message("ok","sent")); 
    } else if (msg.tag == "leave") {
      room->remove_member(&user);
      conn.send(Message("ok","left"));
    } else if (msg.tag == "quit") { // need to figure out what to do on quit
      break;
    } else {
      msg.tag = TAG_ERR;
      msg.data = "Error: invalid command given";
      conn.send(msg);
    }
  }
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  ConnInfo *conn_info = static_cast<ConnInfo*>(arg);

  int client_fd = conn_info->client_fd;
  Server *server = conn_info->server;

  // build Connection
  Connection conn(client_fd);

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Message msg;

  if (!conn.receive(msg)) {
    // client closed connection or error
    std::cerr << "Error: failed connection" << std::endl;
    conn.close();
    return nullptr;
  }

  std::string client;
  std::string username;
  if (msg.tag == TAG_RLOGIN) {
    username = msg.data;
    msg.tag = TAG_OK;
    msg.data = "Login successful";
    conn.send(msg);
    client = "receiver";
  } else if (msg.tag == TAG_SLOGIN) {
    username = msg.data;
    msg.tag = TAG_OK;
    msg.data = "Login successful";
    conn.send(msg);
    client = "sender";
  } else {
    msg.data = TAG_ERR;
    msg.data = "Error: Did not receive login request";
    conn.send(msg);
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (client == "receiver") {
    chat_with_receiver(*server, conn, username);
  } else if (client == "sender") {
    chat_with_sender(*server, conn, username);
  }

  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // TODO: initialize mutex

}

Server::~Server() {
  // TODO: destroy mutex
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::string str = std::to_string(m_port);
  const char* char_port = str.c_str();
  int create_success = open_listenfd(char_port);
  if (create_success > 0) {
    return true;
  }

  return false;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true) {
    int client_fd = ::accept(m_ssock, nullptr, nullptr);
    struct ConnInfo conn_info = {client_fd, this};
    struct ConnInfo *arg;
    arg = &conn_info;
    pthread_t tid;

    ::pthread_create(&tid, nullptr, worker, arg);
  }
}

Room *Server::find_or_create_room(const std::string &room_name) { // figure out
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  std::map<std::string, Room *>::iterator it = m_rooms.find(room_name);
  if (it != m_rooms.end()) {
    return it->second;
  }

  struct Room temp_room = {room_name};
  struct Room *room;
  room = &temp_room;
  return room;
}
