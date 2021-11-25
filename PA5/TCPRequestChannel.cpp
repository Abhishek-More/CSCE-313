#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

#include "TCPRequestChannel.h"
#include "common.h"

void connection_handler (int client_socket){

  char buf [1024];
  while (true){
    if (recv (client_socket, buf, sizeof (buf), 0) < 0){
      perror ("server: Receive failure");    
      exit (0);
    }
    int num = *(int *)buf;
    num *= 2;
    if (num == 0)
      break;
    if (send(client_socket, &num, sizeof (num), 0) == -1){
      perror("send");
      break;
    }
  }
  cout << "Closing client socket" << endl;
  close(client_socket);
}


TCPRequestChannel::TCPRequestChannel(const string host_name, const string port_no) {

  if(host_name == "server") {
    int new_fd;
    struct addrinfo hints, *serv;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, port_no.c_str(), &hints, &serv)) != 0) {
      cerr  << "getaddrinfo: " << gai_strerror(rv) << endl;
      exit(-1);
    }
    if ((this->sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
      perror("server: socket");
      exit(-1);
    }
    if (bind(this->sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
      close(this->sockfd);
      perror("server: bind");
      exit(-1);
    }
    freeaddrinfo(serv); // all done with this structure

    if (listen(this->sockfd, 20) == -1) {
      perror("listen");
      exit(1);
    }

    cout << "server: waiting for connections..." << endl;
    char buf [1024];

  } else {
    struct addrinfo hints, *res;

    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int status;
    //getaddrinfo("www.example.com", "3490", &hints, &res);
    if ((status = getaddrinfo (host_name.c_str(), port_no.c_str(), &hints, &res)) != 0) {
      cerr << "getaddrinfo: " << gai_strerror(status) << endl;
      exit(-1);
    }

    // make a socket:
    this->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (this->sockfd < 0){
      perror ("Cannot create socket");	
      exit(-1);
    }

    // connect!
    if (connect(this->sockfd, res->ai_addr, res->ai_addrlen)<0){
      perror ("Cannot Connect");
      exit(-1);
    }
    freeaddrinfo (res);
  }

}

TCPRequestChannel::TCPRequestChannel(int num) {
  this->sockfd = num;
}

TCPRequestChannel::~TCPRequestChannel() {
  //close(this->sockfd);
}

int TCPRequestChannel::cread(void* msgbuf, int buflen) {
  return recv(this->sockfd, msgbuf, buflen, 0);
}

int TCPRequestChannel::cwrite(void* msgbuf, int msglen) {
  return send(this->sockfd, msgbuf, msglen, 0);
}

int TCPRequestChannel::getfd() {
  return this->sockfd; 
}

