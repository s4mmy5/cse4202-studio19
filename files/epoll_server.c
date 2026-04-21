#define _GNU_SOURCE
#include "inet_header.h"
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50
#define MAX_FDCOUNT 3

void set_non_blocking_io(int);
int init_socket(void);

enum fd_idx {
  in_idx,
  sfd_idx,
  cfd_idx,
};

int main(void) {
  int sfd, cfd, epoll_fd, ret, ready;
  socklen_t peer_addr_size;
  struct sockaddr_in peer_addr;
  struct epoll_event ev;
  struct epoll_event events[MAX_FDCOUNT];
  char buf[PIPE_BUF];
  int fd_cnt = 0;
  int client_cnt = 0;

  // initialize epoll fd
  epoll_fd = epoll_create1(0);
  if (-1 == epoll_fd) {
    err(EXIT_FAILURE, "epoll_create1");
  }

  // initialize STDIN epoll_event
  ev.events = EPOLLIN;
  ev.data.fd = STDIN_FILENO;
  ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
  if (-1 == ret) {
    err(EXIT_FAILURE, "epoll_ctl");
  }
  fd_cnt++;

  // set non blocking readers
  set_non_blocking_io(STDIN_FILENO);

  sfd = init_socket();

  // initialize socket epoll_event
  ev.events = EPOLLIN | EPOLLET;
  ev.data.fd = sfd;
  ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sfd, &ev);
  if (-1 == ret) {
    err(EXIT_FAILURE, "epoll_ctl");
  }
  fd_cnt++;

  // set non blocking readers
  set_non_blocking_io(sfd);

  while ((ready = epoll_wait(epoll_fd, events, fd_cnt, -1)) != -1) {
    for (int i = 0; i < ready; i++) {
      if (events[i].data.fd == STDIN_FILENO) {
        printf("Input available in STDIN\n");
        /* while (read(STDIN_FILENO, buf, PIPE_BUF) > 0); */
      } else if (events[i].data.fd == cfd) {
        if (events[i].events & EPOLLRDHUP) {
          /* printf("DEBUG: client POLLHUP triggered\n"); */
          printf("Client %d has closed the connection\n", client_cnt);
          ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cfd, NULL);
          if (-1 == ret) {
            err(EXIT_FAILURE, "epoll_ctl");
          }

          if (-1 == close(cfd))
            err(EXIT_FAILURE, "fclose");
          fd_cnt -= 1;
        } else if (events[i].events & EPOLLIN) {
          printf("Input available from Client %d\n", client_cnt);
        }

      } else if (events[i].data.fd == sfd) {
        if (events[i].events & EPOLLIN) {
          cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);

          if (cfd == -1)
            err(EXIT_FAILURE, "accept");

          client_cnt++;

          // initialize socket pollfd
          ev.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
          ev.data.fd = cfd;
          ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, cfd, &ev);
          if (-1 == ret) {
            err(EXIT_FAILURE, "epoll_ctl");
          }
          fd_cnt++;

          // set non blocking readers
          set_non_blocking_io(sfd);
        }
      }
    }
  }

  if (close(STDIN_FILENO) == -1) {
    err(EXIT_FAILURE, "fclose");
  }

  if (close(sfd) == -1)
    err(EXIT_FAILURE, "close");

  return 0;
}

void set_non_blocking_io(int fd) {
  int flags;

  flags = fcntl(fd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);
}

int init_socket(void) {
  int sfd;
  struct sockaddr_in my_addr;

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sfd == -1)
    err(EXIT_FAILURE, "socket");

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT_NUM);
  my_addr.sin_addr.s_addr = CT_ADDR;

  if (bind(sfd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    err(EXIT_FAILURE, "bind");

  if (listen(sfd, LISTEN_BACKLOG) == -1)
    err(EXIT_FAILURE, "listen");

  return sfd;
}
