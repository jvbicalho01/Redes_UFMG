#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

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

// struct para guardar as coordenadas do cliente
typedef struct {
  double latitude;
  double longitude;
} Coordinate;

// Inicializando as coordenadas do cliente
Coordinate coordServ = {-19.8657, -43.9874};

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  // TODO: passar argv[2] e argv[3]
  if (0 != addrparse(argv[2], argv[3], &storage)) {
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

  size_t count;
  unsigned total = 0;

  int clientConnected = 0;

  while (1) {
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("0 - Sair\n");
    printf("1 - Solicitar corrida\n");

    if (clientConnected == 1) {
      printf("0 - Sair\n");
      printf("1 - Solicitar corrida\n");
      clientConnected = 0;
    }
    fgets(buf, BUFSZ - 1, stdin);

    // if (fgets(buf, BUFSZ - 1, stdin) == NULL) {
    //   break;
    // }

    // envia se o cliente pediu ou não a corrida
    size_t countSend = send(s, buf, strlen(buf), 0);

    if (countSend != strlen(buf)) {
      printf("LOGEXIT SERVER SEND");
      logexit("send");
    }

    // trata o caso onde o cliente não solicita a corrida
    if (strncmp(buf, "0", 1) == 0) {
      printf("Saiu da corrida\n");

      memset(buf, 0, BUFSZ);  // clears buffer

      // recebe do servidor a "confirmação" de cancelamento
      size_t countRecv = recv(s, buf + total, BUFSZ - total, 0);
      close(s);
      break;
    }

    // trata o caso onde o cliente solicita a corrida
    else if (strncmp(buf, "1", 1) == 0) {


      memset(buf, 0, BUFSZ);

      // recebe se o motorista aceitou ou recusou a corrida
      count = recv(s, buf + total, BUFSZ - total, 0);

      // trata caso o motorista tenha recusado
      if (strncmp(buf, "0", 1) == 0) {
        printf("Não foi encontrado um motorista\n");
        // close(s);

        // break;
        // clientConnected = 1;
      }
      // trata caso o motorista tenha aceitado
      else if (strncmp(buf, "1", 1) == 0) {
        printf("Aceitou a corrida\n");
        close(s);
        break;
      }
    }
  }

  // Fechar o socket do cliente
  close(s);

  return 0;
}