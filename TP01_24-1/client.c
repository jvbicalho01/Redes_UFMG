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

// struct Coordinate {
//   double latitude;
//   double longitude;
// };

// Inicializando as coordenadas do cliente
Coordinate coordCli = {-19.8657, -43.9874};
// struct Coordinate coordCli = {-19.8657, -43.9874};

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
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
    // pega da entrada o que o cliente digitou (0 para sair e 1 para solicitar
    // corrida)
    fgets(buf, BUFSZ - 1, stdin);

    // if (fgets(buf, BUFSZ - 1, stdin) == NULL) {
    //   break;
    // }

    // envia para o servidor se o cliente pediu ou não a corrida
    size_t countSend = send(s, buf, strlen(buf), 0);

    if (countSend < 0) {
      printf("LOGEXIT SERVER SEND");
      logexit("send");
    }

    // trata o caso onde o cliente não solicita a corrida
    if (strncmp(buf, "0", 1) == 0) {
      // printf("Saiu da corrida\n");

      close(s);
      break;
    }

    // trata o caso onde o cliente solicita a corrida
    else if (strncmp(buf, "1", 1) == 0) {
      memset(buf, 0, BUFSZ);

      // recebe do servidor se o motorista aceitou ou recusou a corrida
      size_t countRecv = recv(s, buf, BUFSZ, 0);

      if (countRecv < 0) {
        // printf("LOGEXIT SERVER SEND");
        logexit("recv");
      }

      // trata caso o motorista tenha recusado
      if (strncmp(buf, "0", 1) == 0) {
        printf("Não foi encontrado um motorista\n");
      }
      // trata caso o motorista tenha aceitado
      else if (strncmp(buf, "1", 1) == 0) {
        // envia as coordenadas para o servidor
        size_t countSend = send(s, &coordCli, sizeof(Coordinate), 0);

        if (countSend < 0) {
          printf("LOGEXIT SERVER SEND");
          logexit("send");
        }

        memset(buf, 0, BUFSZ);
        double recvDistance;
        // recebe do servidor a distancia do motorista a cada 2 segundos (após
        // um recv a distancia é diminuida em 400m)
        while (recv(s, &recvDistance, sizeof(double), 0) > 0) {
          printf("Motorista a %.0fm\n", recvDistance);
          if (recvDistance - 400 < 0) {
            sleep(2);
            printf("O motorista chegou!\n");
            close(s);
            break;
          }
        }

        close(s);
        break;
      }
    }
  }

  // Fechar o socket do cliente
  close(s);

  return 0;
}