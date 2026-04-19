#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <limits.h>

#define QUIT_CMD "quit"

int main(int argc, char *argv[]) {
  FILE *in;
  fd_set readfds, writefds;
  int ready, nfds, fd, numRead, j;
  char buf[PIPE_BUF];

  for (;;) {
    /* Process remaining arguments to build file descriptor sets */
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    nfds = STDIN_FILENO + 1;
    FD_SET(STDIN_FILENO, &readfds);

    /* We've built all of the arguments; now call select() */
    ready = select(nfds, &readfds, &writefds, NULL, NULL);
    /* Ignore exceptional events */
    if (ready == -1)
      err(EXIT_FAILURE, "select");

    /* Display results of select() */
    printf("Input available in STDIN\n");
    while (fgets(buf, PIPE_BUF, STDIN_FILENO)) {
      printf("Input received: %s\n", buf);
      if (strcmp(QUIT_CMD, buf) < 0) {
        exit(EXIT_SUCCESS);
      }
    }
  }
  exit(EXIT_SUCCESS);
}
