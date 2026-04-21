#include "inet_header.h"
#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int sfd;
  unsigned int write_cnt, in;
  socklen_t peer_addr_size;
  struct sockaddr_in my_addr, peer_addr;
  FILE *s_fp;
  char client_host[NI_MAXHOST];
  char server_host[NI_MAXHOST];
  char server_serv[NI_MAXSERV];
  char buf[PIPE_BUF];

  if (argc > 2) {
    err(EXIT_FAILURE, "Invalid argument count");
  }

  sfd = socket(AF_INET, SOCK_STREAM, 0);
  if (-1 == sfd)
    err(EXIT_FAILURE, "socket");

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(PORT_NUM);
  if (0 == inet_aton(SV_ADDR, &my_addr.sin_addr))
    err(EXIT_FAILURE, "inet_aton");

  if (-1 == connect(sfd, (struct sockaddr *)&my_addr, sizeof(my_addr)))
    err(EXIT_FAILURE, "connect");

  printf("Connection established\n");
  s_fp = fdopen(sfd, "r");
  while (NULL != fgets(buf, PIPE_BUF, s_fp)) {
    printf("%s", buf);
  }

  if (-1 == fclose(s_fp))
    err(EXIT_FAILURE, "fclose");

  return 0;
}
