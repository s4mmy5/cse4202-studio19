#define _GNU_SOURCE
#include "inet_header.h"
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50

void set_non_blocking_io(int);
int init_socket(void);

enum fd_idx {
  in_idx,
  sfd_idx,
  cfd_idx,
};

int main(void) {
  int sfd, cfd;
  socklen_t peer_addr_size;
  struct sockaddr_in peer_addr;
  FILE *c_fp, *in_fp;
  char buf[PIPE_BUF];
  struct pollfd pollFd[3] = {0};
  int fd_cnt = 0;
  int client_cnt = 0;

  // initialize STDIN pollfd
  pollFd[in_idx].fd = STDIN_FILENO;
  pollFd[in_idx].events = POLLIN;
  fd_cnt += 1;

  // open stdin file stream in advance
  in_fp = fdopen(STDIN_FILENO, "r");

  // set non blocking readers
  set_non_blocking_io(STDIN_FILENO);

  sfd = init_socket();

  // initialize socket pollfd
  pollFd[sfd_idx].fd = sfd;
  pollFd[sfd_idx].events = POLLIN;
  fd_cnt += 1;

  // set non blocking readers
  set_non_blocking_io(sfd);

  while (poll(pollFd, fd_cnt, -1) != -1) {
    // check for stdin input
    if (pollFd[in_idx].revents & POLLIN) {
      while (NULL != fgets(buf, PIPE_BUF, in_fp)) {
        printf("Input received from STDIN: %s", buf);
        if (!strcmp(QUIT_CMD, buf)) {
          printf("Quit command received\n");
          exit(EXIT_SUCCESS);
        }
        printf("\n");
      }
    }

    if (fd_cnt > 2 && pollFd[cfd_idx].revents & POLLIN) {
      /* printf("DEBUG: client POLLIN triggered\n"); */
      while (NULL != fgets(buf, PIPE_BUF, c_fp)) {
        printf("Client %d: %s", client_cnt, buf);
        if (!strcmp(QUIT_CMD, buf)) {
          printf("Quit command received\n");
          exit(EXIT_SUCCESS);
        }
      }
    }

    if (fd_cnt > 2 && pollFd[cfd_idx].revents & POLLRDHUP) {
      /* printf("DEBUG: client POLLHUP triggered\n"); */
      if (-1 == fclose(c_fp))
        err(EXIT_FAILURE, "fclose");
      fd_cnt -= 1;
    }

    if (pollFd[sfd_idx].revents & POLLIN) {
      /* printf("DEBUG: socket POLLIN triggered\n"); */
      cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);

      if (cfd == -1)
        err(EXIT_FAILURE, "accept");

      client_cnt++;

      // initialize socket pollfd
      pollFd[cfd_idx].fd = cfd;
      pollFd[cfd_idx].events = POLLRDHUP | POLLIN;
      fd_cnt += 1;

      // set non blocking readers
      set_non_blocking_io(sfd);

      // prematurely create FILE *
      c_fp = fdopen(cfd, "r");

      if (NULL == c_fp)
        err(EXIT_FAILURE, "fdopen");

      setbuf(c_fp, NULL);
    }
  }

  if (fclose(in_fp) == -1) {
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
