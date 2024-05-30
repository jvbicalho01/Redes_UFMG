#include "common.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
// #include <cmath>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void usage(int argc, char** argv) {
  printf("usage: %s <ipv4|ipv6> <server port>\n", argv[0]);
  printf("exemple: %s ipv4 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

char* frases_senhorDosAneis[] = {
    "Um anel para a todos governar",
    "Na terra de Mordor onde as sombras se deitam",
    "Não é o que temos, mas o que fazemos com o que temos",
    "Não há mal que sempre dure",
    "O mundo está mudando, senhor Frodo",
};

char* frases_poderosoChefao[] = {
    "Vou fazer uma oferta que ele não pode recusar",
    "Mantenha seus amigos por perto e seus inimigos mais perto ainda",
    "É melhor ser temido que amado", "A vingança é um prato que se come frio",
    "Nunca deixe que ninguém saiba o que você está pensando"};

char* frases_clubeDaLuta[] = {
    "Primeira regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "Segunda regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "O que você possui acabará possuindo você",
    "É apenas depois de perder tudo que somos livres para fazer qualquer coisa",
    "Escolha suas lutas com sabedoria"};

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  // Cria estrutura que vai representar o endereço e porta do servidor
  struct sockaddr_storage storage;
  if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
    usage(argc, argv);
  }

  // Inicializa soquete do servidor
  int s;
  s = socket(storage.ss_family, SOCK_DGRAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  // Faz com que seja possivel reutilizar o endereço IP para novas conexões no
  // socket do servidor
  int enable = 1;
  if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
    logexit("setsockopt");
  }

  // Atrela soquete do servidor ao endereço IP
  struct sockaddr* addr = (struct sockaddr*)(&storage);
  if (0 != bind(s, addr, sizeof(storage))) {
    logexit("bind");
  }

  // // Endereço IP do cliente
  // struct sockaddr_storage cstorage;
  // struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
  // socklen_t caddrlen = sizeof(cstorage);

  // char client_option[BUFSZ];
  // char response[BUFSZ];

  // // recebe do cliente a opção escolhida
  // size_t coutRecv =
  //     recvfrom(s, &client_option, sizeof(client_option), 0, caddr,
  //     &caddrlen);

  // if (coutRecv < 0) {
  //   logexit("recvfrom");
  // }

  // if (strncmp(client_option, "1", 1) == 0) {
  //   for (int i = 0; i < 5; i++) {
  //     memset(response, 0, BUFSZ);
  //     strcpy(response, frases_senhorDosAneis[i]);
  //     sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
  //   }

  // } else if (strncmp(client_option, "2", 1) == 0) {
  //   for (int i = 0; i < 5; i++) {
  //     memset(response, 0, BUFSZ);
  //     strcpy(response, frases_poderosoChefao[i]);
  //     sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
  //   }

  // } else if (strncmp(client_option, "3", 1) == 0) {
  //   for (int i = 0; i < 5; i++) {
  //     memset(response, 0, BUFSZ);
  //     strcpy(response, frases_clubeDaLuta[i]);
  //     sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
  //   }
  // } else {
  //   // close(s);
  //   logexit("opção inválida!");
  // }

  while (1) {
    // Endereço IP do cliente
    struct sockaddr_storage cstorage;
    struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    char client_option[BUFSZ];
    char response[BUFSZ];

    // recebe do cliente a opção escolhida
    size_t coutRecv =
        recvfrom(s, &client_option, sizeof(client_option), 0, caddr, &caddrlen);

    if (coutRecv < 0) {
      logexit("recvfrom");
    }

    if (strncmp(client_option, "1", 1) == 0) {
      for (int i = 0; i < 5; i++) {
        memset(response, 0, BUFSZ);
        strcpy(response, frases_senhorDosAneis[i]);
        sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
      }

    } else if (strncmp(client_option, "2", 1) == 0) {
      for (int i = 0; i < 5; i++) {
        memset(response, 0, BUFSZ);
        strcpy(response, frases_poderosoChefao[i]);
        sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
      }

    } else if (strncmp(client_option, "3", 1) == 0) {
      for (int i = 0; i < 5; i++) {
        memset(response, 0, BUFSZ);
        strcpy(response, frases_clubeDaLuta[i]);
        sendto(s, response, BUFSZ - 1, 0, caddr, caddrlen);
      }
    } else {
      // close(s);
      logexit("opção inválida!");
    }
  }

  // fecha o socket do servidor
  close(s);

  return 0;
}