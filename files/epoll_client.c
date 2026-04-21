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
  struct sockaddr_in my_addr;
  FILE *s_fp;

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
  s_fp = fdopen(sfd, "w");
  if (NULL == s_fp) {
    err(EXIT_FAILURE, "fdopen");
  }

  fprintf(s_fp, "%s\n", "Message 1");
  fprintf(s_fp, "%s\n", "Message 2");
  if (argc > 1 && !strcmp(argv[1], "quit")) {
    printf("Issuing quit command\n");
    fprintf(s_fp, "quit\n");
  }

  if (-1 == fclose(s_fp))
    err(EXIT_FAILURE, "fclose");

  return 0;
}
