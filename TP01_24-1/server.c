#include "common.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <cmath>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

#define M_PI 3.14159265358979323846

double haversine(double lat1, double long1, double lat2, double long2) {
  // distance between latitudes
  // and longitudes
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLong = (long2 - long1) * M_PI / 180.0;

  // convert to radians
  lat1 = (lat1)*M_PI / 180.0;
  lat2 = (lat2)*M_PI / 180.0;

  // apply formulae
  double a =
      pow(sin(dLat / 2), 2) + pow(sin(dLong / 2), 2) * cos(lat1) * cos(lat2);
  double rad = 6371;
  double c = 2 * asin(sqrt(a));
  return rad * c * 1000;
}

// struct para guardar as coordenadas do motorista
typedef struct {
  double latitude;
  double longitude;
} Coordinate;

// struct Coordinate {
//   double latitude;
//   double longitude;
// };

// Inicializando as coordenadas do motorista
Coordinate coordServ = {-19.9227, -43.9451};
// struct Coordinate coordServ = {-19.9227, -43.9451};

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
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("exemple: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  struct sockaddr_storage storage;
  if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
    usage(argc, argv);
  }

  int s;
  s = socket(storage.ss_family, SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  int enable = 1;
  if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
    logexit("setsockopt");
  }

  struct sockaddr* addr = (struct sockaddr*)(&storage);

  if (0 != bind(s, addr, sizeof(storage))) {
    logexit("bind");
  }

  if (0 != listen(s, 10)) {
    logexit("listen");
  }

  char addrstr[BUFSZ];
  addrtostr(addr, addrstr, BUFSZ);

  struct sockaddr_storage cstorage;
  struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
  socklen_t caddrlen = sizeof(cstorage);

  printf("Aguardando solicitação.\n");

  // Espera uma conexão do cliente
  int csock = accept(s, caddr, &caddrlen);
  if (csock == -1) {
    logexit("accept");
  }

  char caddrstr[BUFSZ];
  addrtostr(caddr, caddrstr, BUFSZ);
  // printf("client connected\n");

  int clientConnected = 0;
  int clientExitConnection = 0;

  while (1) {
    if (clientConnected == 1) {
      printf("Aguardando solicitação.\n");
      // espera uma conexão do cliente
      csock = accept(s, caddr, &caddrlen);
      if (csock == -1) {
        logexit("accept");
      }

      char caddrstr[BUFSZ];
      addrtostr(caddr, caddrstr, BUFSZ);
      // printf("client connected\n");

      clientConnected = 0;
    }

    if (clientExitConnection == 1) {
      // printf("Aguardando solicitação.\n");
      // espera uma conexão com o cliente
      csock = accept(s, caddr, &caddrlen);
      if (csock == -1) {
        logexit("accept");
      }

      char caddrstr[BUFSZ];
      addrtostr(caddr, caddrstr, BUFSZ);
      // printf("client connected\n");

      clientConnected = 0;
    }

    if (clientConnected == 2) {
      printf("Aguardando solicitação.\n");

      clientConnected = 0;
    }

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    Coordinate coordRecv;

    // recebe do cliente se solicitou ou não pediu uma corrida
    size_t countRecv = recv(csock, buf, BUFSZ - 1, 0);

    if (countRecv < 0) {
      // printf("LOGEXIT SERVER SEND");
      logexit("recv");
    }

    // verifica o caso onde o cliente não pediu a corrida
    if (strncmp(buf, "0", 1) == 0) {
      // clientConnected = 1;
      clientExitConnection = 1;
    }

    // verifica o caso onde o cliente solicitou a corrida
    else if (strncmp(buf, "1", 1) == 0) {
      printf("Corrida disponível:\n");
      printf("0 - Recusar\n");
      printf("1 - Aceitar\n");
      // pega da entrada o que o motorista digitou (0 para recusar a corrida e 1
      // para aceitar a corrida)
      fgets(buf, BUFSZ - 1, stdin);
      // envia para o cliente se o motorista aceitou ou não a corrida
      size_t countSend = send(csock, buf, strlen(buf) + 1, 0);
      if (countSend < 0) {
        logexit("send");
      }
      // trata caso o motorista tenha recusado
      if (strncmp(buf, "0", 1) == 0) {
        clientConnected = 2;
      }
      // trata caso o motorista tenha aceitado
      else if (strncmp(buf, "1", 1) == 0) {
        // recebe as coordenadas do cliente
        size_t countRecv = recv(csock, &coordRecv, sizeof(Coordinate), 0);

        if (countRecv < 0) {
          // printf("LOGEXIT SERVER SEND");
          logexit("recv");
        }

        // calcula a distancia entre as coordenadas
        double distance = haversine(coordRecv.latitude, coordRecv.longitude,
                                    coordServ.latitude, coordServ.longitude);

        // envia a distancia para o cliente a cada 2 segundos e reduz em 400m
        // após cada envio
        while (distance > 0) {
          memset(buf, 0, BUFSZ);
          size_t countSend = send(csock, &distance, sizeof(double), 0);
          if (countSend < 0) {
            printf("LOGEXIT SERVER SEND");
            logexit("send");
          }
          distance -= 400;
          sleep(2);
          memset(buf, 0, BUFSZ);
        }
        printf("O motorista chegou!\n");
        clientConnected = 1;
      }
    }
  }

  // fecha a conexão com o cliente
  close(csock);
  // Fechar o socket do servidor
  close(s);

  return 0;
}
