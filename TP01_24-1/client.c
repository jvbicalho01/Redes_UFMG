#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

// send
// 1° parametro: socket
// 2° parametro: dado, onde está armazenado
// 3° parametro: tamanho do dado numero de bytes
// 4° parametro:

// recv
// 1° parametro: socket
// 2° parametro: dado, onde vai armazenar
// 3° parametro: tamanho do dado - numero de bytes
// 4° parametro:

void usage(int argc, char** argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("exemple: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (0 != addrparse(argv[1], argv[2], &storage)) {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  struct sockaddr* addr = (struct sockaddr*)(&storage);

  if (0 != connect(s, addr, sizeof(storage))) {
    logexit("connect");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);
  printf("connected to %s\n", addrstr);

  char buf[BUFSZ];
  memset(buf, 0, BUFSZ);
  printf("> ");
  fgets(buf, BUFSZ - 1, stdin);

  size_t count;
  unsigned total = 0;

  while (1) {
    // printf(">");
    // fgets(buf, BUFSZ - 1, stdin);

    count = send(s, buf, strlen(buf), 0);  // doesn't send '\0' character
    if (count != strlen(buf)) {
      logexit("send");
    }

    memset(buf, 0, BUFSZ);  // clears buffer

    count = recv(s, buf + total, BUFSZ - total, 0);
    if (count == 0) {
      // Connection terminated
      break;
    }
    total += count;
    memset(buf, 0, BUFSZ);
  }

  close(s);

  printf("received %u bytes\n", total);
  // printa a mensagem
  puts(buf);

  exit(EXIT_SUCCESS);
}