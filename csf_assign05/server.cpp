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
  User *user = new User(username);
  if (!conn.receive(msg) || msg.tag != TAG_JOIN) { // handle case where server does not receive join message from client
    msg.tag = TAG_ERR;
    msg.data = "Error: Did not receive join request.";
    conn.send(msg);
    delete user;
    return;
  }
  Room *room = server.find_or_create_room(msg.data);
  room->add_member(user);
  msg.tag = TAG_OK;
  msg.data = "Join successful";
  conn.send(msg);

  // retrieve new messsasges form server while connection is open and message queue is non-empty
  MessageQueue &userqueue = user->mqueue;
  Message *new_msg;
  while (conn.is_open()) {
    new_msg = userqueue.dequeue();
    if (new_msg == nullptr) {
      continue;
    }
    conn.send(*new_msg);
    delete new_msg;
  }
  room->remove_member(user);
  delete user;
}

void chat_with_sender(Server &server, Connection &conn, const std::string username) {
  Message msg;
  User   *user = new User(username);
  Room   *current_room = nullptr;

  while (conn.receive(msg)) {
    if (msg.tag == TAG_JOIN) {
      // leave room user is currently in
      if (current_room) {
        current_room->remove_member(user);
      }

      current_room = server.find_or_create_room(msg.data);
      current_room->add_member(user);

      msg.tag = TAG_OK;
      msg.data = "Joined Room.";
      if (!conn.send(msg)) {
        break;
      }
    
    } else if (msg.tag == TAG_SENDALL) {
      if (!current_room) {
        msg.tag = TAG_ERR;
        msg.data = "Error: Not in a room. JOIN first.";
        conn.send(msg);
      } else {
        current_room->broadcast_message(username, msg.data);
        msg.tag = TAG_OK;
        msg.data = "Message sent.";
        if (!conn.send(msg)) {
          break;
        }
      }

    } else if (msg.tag == TAG_LEAVE) {
      if (current_room) {
      current_room->remove_member(user);
      current_room = nullptr;
      msg.tag = TAG_OK;
      msg.data = "Left current rooom.";
      conn.send(msg);
      } else {
        msg.tag = TAG_ERR;
        msg.data = "Error: Not in a room.";
        if (!conn.send(msg)) {
          break;
        }
      }

    } else if (msg.tag == TAG_QUIT) {
      msg.tag = TAG_OK;
      msg.data = "Quiting...";
      if (!conn.send(msg)) {
        break;
      }
      break;   // exit the loop

    } else {
      msg.tag = TAG_ERR;
      msg.data = "Error: Invalid command.";
      if (!conn.send(msg)) {
        break;
      }
    }
  }
  // check for message length in connecttion.cpp
  if (conn.m_last_result == INVALID_MSG) {
    msg.tag = TAG_ERR;
    msg.data = "Message is too long.";
    conn.send(msg);
  }

  // cleanup
  if (current_room) {
      current_room->remove_member(user);
  }
  delete user;
}

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  ConnInfo *conn_info = static_cast<ConnInfo*>(arg);

  int client_fd = conn_info->client_fd;
  Server *server = conn_info->server;
  delete conn_info;
  std::string username;

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

    // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  
  if (msg.tag == TAG_RLOGIN) {
  username = msg.data;
  msg.tag = TAG_OK;
  msg.data = "Login successful";
  conn.send(msg);
  chat_with_receiver(*server, conn, username);
} else if (msg.tag == TAG_SLOGIN) {
  username = msg.data;
  msg.tag = TAG_OK;
  msg.data = "Login successful";
  conn.send(msg);
  chat_with_sender(*server, conn, username);
} else {
  msg.data = "Error: Did not receive login request";
  conn.send(msg);
}

  conn.close();
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
  ::pthread_mutex_init(&m_lock, NULL);
}

Server::~Server() {
  // TODO: destroy mutex
  for (std::map<std::string,Room*>::iterator it = m_rooms.begin();
    it != m_rooms.end();
    ++it)
  {
    delete it->second;
  }
  m_rooms.clear();

  ::pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::string str = std::to_string(m_port);
  const char* char_port = str.c_str();
  int create_success = open_listenfd(char_port);
  if (create_success > 0) {
    m_ssock = create_success;
    return true;
  }
  return false;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while (true) {
    int client_fd = ::accept(m_ssock, nullptr, nullptr);
    struct ConnInfo *conn_info = new ConnInfo{ client_fd, this };
    pthread_t tid;

    if (::pthread_create(&tid, NULL, worker, conn_info) != 0) {
      std::cerr << "pthread_create failed" << std::endl;
      exit(1);
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) { // figure out
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  Guard lock_guard(m_lock);

  std::map<std::string, Room *>::iterator it = m_rooms.find(room_name);
  if (it != m_rooms.end()) {
    return it->second;
  }

  struct Room *room = new Room(room_name);
  m_rooms[room_name] = room;
  return room;
}
