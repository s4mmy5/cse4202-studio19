#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define QUIT_CMD "quit\n"

int main(int argc, char *argv[]) {
  FILE *in;
  fd_set readfds;
  int ready, nfds, fd, numRead, j, flags;
  char buf[PIPE_BUF];

  flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  flags |= O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, flags);
  in = fdopen(STDIN_FILENO, "r");
  for (;;) {
    /* Process remaining arguments to build file descriptor sets */
    FD_ZERO(&readfds);
    nfds = STDIN_FILENO + 1;
    FD_SET(STDIN_FILENO, &readfds);

    /* We've built all of the arguments; now call select() */
    ready = select(nfds, &readfds, NULL, NULL, NULL);
    /* Ignore exceptional events */
    if (ready == -1)
      err(EXIT_FAILURE, "select");

    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      /* Display results of select() */
      printf("Input available in STDIN\n");
      while (0 < fgets(buf, PIPE_BUF, in)) {
        printf("Input received: %s", buf);
        if (!strcmp(QUIT_CMD, buf)) {
          printf("Quit command received\n");
          exit(EXIT_SUCCESS);
        }
        printf("\n");
      }
    }
  }
  exit(EXIT_SUCCESS);
}
