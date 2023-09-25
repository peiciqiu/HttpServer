/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <cstdio>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <cstring>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

namespace searchserver {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::bind_and_listen(int *listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"
  // NOTE: You only have to support IPv6, you do not have to support IPv4
  // TODO: implement
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET6;       // IPv6 (also handles IPv4 clients)
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use an address we can bind to a socket and accept client connections on
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  struct addrinfo *result;

  char port_arr [100];
  sprintf(port_arr, "%d", port_);
  int res = getaddrinfo(nullptr, port_arr, &hints, &result);
  if (res != 0) {
    std::cerr << "getaddrinfo() failed: ";
    std::cerr << gai_strerror(res) << std::endl;
    *listen_fd = listen_sock_fd_;
    return false;
  }

  for (struct addrinfo *rp = result; rp != nullptr; rp = rp->ai_next) {
    listen_sock_fd_ = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listen_sock_fd_ == -1) {
      std::cerr << "socket() failed: " << strerror(errno) << std::endl;
      listen_sock_fd_ = 0;
      continue;
    }

    int optval = 1;
    setsockopt(listen_sock_fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(listen_sock_fd_, rp->ai_addr, rp->ai_addrlen) == 0) {
      *listen_fd = listen_sock_fd_;
      break;
    }
    close(listen_sock_fd_);
    listen_sock_fd_ = -1;
  }

  freeaddrinfo(result);
  if (listen_sock_fd_ == -1) {
    std::cerr << "Couldn't bind to any addresses." << std::endl;
    *listen_fd = listen_sock_fd_;
    return false;
  }

  if (listen(listen_sock_fd_, SOMAXCONN) != 0) {
    std::cerr << "Failed to mark socket as listening: ";
    std::cerr << strerror(errno) << std::endl;
    close(listen_sock_fd_);
    *listen_fd = listen_sock_fd_;
    return false;
  }

  sleep(20);
  *listen_fd = listen_sock_fd_;
  return true;

}

bool ServerSocket::accept_client(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dns_name,
                          std::string *server_addr,
                          std::string *server_dns_name) const {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // TODO: implement
  int listen_fd = listen_sock_fd_;
  if (listen_fd <= 0) {
    std::cerr << "Couldn't bind to any addresses." << std::endl;
    return false;
  }

  while (true) {
    struct sockaddr_storage caddr;
    socklen_t caddr_len = sizeof(caddr);
    int client_fd = accept(listen_fd, reinterpret_cast<struct sockaddr *>(&caddr), &caddr_len);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK)){
        continue;
      }
      std::cerr << "Failure on accept: " << strerror(errno) << std::endl;
      return false;
    }
    else {
      *accepted_fd = client_fd;
      *client_port = port_;

      struct sockaddr *addr = reinterpret_cast<struct sockaddr *>(&caddr);
      struct sockaddr_in6 *in6 = reinterpret_cast<struct sockaddr_in6 *>(addr);
      
      char astring[INET6_ADDRSTRLEN];
      char hostname[1024]; 
      inet_ntop(AF_INET6, &(in6->sin6_addr), astring, INET6_ADDRSTRLEN);
      getnameinfo(addr, sizeof(caddr), hostname , 1024, nullptr, 0, 0);
      *client_addr = astring;
      *client_dns_name = hostname;

      char addrbuf[INET_ADDRSTRLEN];
      char hname[1024];
      struct sockaddr_in6 srvr;
      socklen_t srvrlen = sizeof(srvr);
      getsockname(client_fd, (struct sockaddr *) &srvr, &srvrlen);
      inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
      getnameinfo((const struct sockaddr *) &srvr, srvrlen, hname, 1024, nullptr, 0, 0);
      *server_addr = addrbuf;      
      *server_dns_name = hname;
      break;
    }
  }

  close(listen_fd);
  return true;
}


}  // namespace searchserver

