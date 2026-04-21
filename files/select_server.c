#include "inet_header.h"
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#define LISTEN_BACKLOG 50

int main(void) {
  int sfd, cfd;
  unsigned int c_cnt;
  struct fd_set readfds;
  int nfds, ready, flags;
  socklen_t peer_addr_size;
  struct sockaddr_in my_addr, peer_addr;
  FILE *c_fp, *in_fp;
  char server_host[NI_MAXHOST];
  char client_host[NI_MAXHOST];
  char client_serv[NI_MAXSERV];
  char buf[PIPE_BUF];
  struct timeval tv;

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

  // open stdin file stream in advance
  in_fp = fdopen(STDIN_FILENO, "r");

  // set non blocking readers
  flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flags);

  flags = fcntl(sfd, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(sfd, F_SETFL, flags);

  peer_addr_size = sizeof(peer_addr);
  c_cnt = 0;
  for (;;) {
    // select busy loop
    for (;;) {
      FD_ZERO(&readfds);
      nfds = sfd + 1;
      FD_SET(STDIN_FILENO, &readfds);
      FD_SET(sfd, &readfds);
      ready = select(nfds, &readfds, NULL, NULL, NULL);

      /* Ignore exceptional events */
      if (ready == -1)
        err(EXIT_FAILURE, "select");

      if (FD_ISSET(STDIN_FILENO, &readfds)) {
        /* Display results of select() */
        printf("Input available in STDIN\n");
        while (0 < fgets(buf, PIPE_BUF, in_fp)) {
          printf("Input received: %s", buf);
          if (!strcmp(QUIT_CMD, buf)) {
            printf("Quit command received\n");
            exit(EXIT_SUCCESS);
          }
          printf("\n");
        }
      }

      if (FD_ISSET(sfd, &readfds)) {
        cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_size);

        if (cfd == -1)
          err(EXIT_FAILURE, "accept");

        /* Code to deal with incoming connection(s)...  */
        c_fp = fdopen(cfd, "w");
        setbuf(c_fp, NULL);

        if (NULL == c_fp)
          err(EXIT_FAILURE, "fdopen");

        printf("Client connection #%d accepted\n", c_cnt);

        gethostname(client_host, NI_MAXHOST);
        fprintf(c_fp, "Server FQDN: %s\n", client_host);

        gettimeofday(&tv, NULL);
        fprintf(c_fp, "Server time of day: %ld.%06ld\n", tv.tv_sec, tv.tv_usec);

        if (-1 == fclose(c_fp))
          err(EXIT_FAILURE, "fclose");

        c_cnt++;
      }
    }
  }

out:
  if (-1 == fclose(c_fp))
    err(EXIT_FAILURE, "fclose");

  if (close(sfd) == -1)
    err(EXIT_FAILURE, "close");

  return 0;
}
