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

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {
void chat_with_receiver(Connection &conn, const std::string username) {
  Message msg;
  User user(username);
  conn.receive(msg);
  if (msg.tag == TAG_JOIN) {
    Room room(msg.data);
    room.add_member(&user);
    msg.tag = TAG_OK;
    msg.data = "Join successful";
    conn.send(msg);
  } else { // want to handle case where server does not receive join message from client
    exit( 1 );
  }

  while (conn.receive(msg)) {
    // wait for another message sent into the room (from senders)
    // figure out message delivery
    size_t i = 0;
    std::string room;
    std::string sender;
    std::string message;

    // loop through received msg data to get room
    while (msg.data[i] != ':') {
      room += msg.data[i];
      i++;
    }
    i++;

    // loop through received msg data to get sender
    while (msg.data[i] != ':') {
      sender += msg.data[i];
      i++;
    }
    i++;

    // loop through received msg data to get actual message
    while (i < msg.data.size()) {
      message += msg.data[i];
      i++;
    }

    Message out("delivery", room + ":" + sender + ":" + message);
    conn.send(out);
  }
}

void chat_with_sender(Connection &conn, const std::string username) {
  Message msg;
  User user(username);
  conn.receive(msg);
  if (msg.tag == TAG_JOIN) {
    Room room(msg.data);
    room.add_member(&user);
    msg.tag = TAG_OK;
    msg.data = "Join successful";
    conn.send(msg);
  } else { // want to handle case where server does not receive join message from client
    exit( 1 );
  }

  while (conn.receive(msg)) {
    if (msg.tag == "sendall") {
      // need to figure out how to broadcast to all receivers in this room
      conn.send(Message("ok","sent")); 
    } else if (msg.tag == "leave") {
      // need to figure out how to remove from room, send back ok

      conn.send(Message("ok","left"));
    } else if (msg.tag == "quit") { // need to figure out what to do on quit
      break;
    }
  }
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  int client_fd = static_cast<int>(reinterpret_cast<intptr_t>(arg));

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
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  if (client == "receiver") {
    chat_with_receiver(conn, username);
  } else if (client == "sender") {
    chat_with_sender(conn, username);
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
    void *arg = reinterpret_cast<void*>(static_cast<intptr_t>(client_fd));
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

  Room temp_room(room_name);
  Room *room = &temp_room;
  return room;
}
