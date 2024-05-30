#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void usage(int argc, char** argv) {
  printf("usage: %s <server IP> <server port>\n", argv[0]);
  printf("exemple: %s 127.0.0.1 51511\n", argv[0]);
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  if (argc < 3) {
    usage(argc, argv);
  }

  // Cria estrutura para representar o endereço e porta do servidor
  struct sockaddr_storage storage;
  if (0 != addrparse(argv[2], argv[3], &storage)) {
    usage(argc, argv);
  }

  size_t addr_storage_size = storage.ss_family == AF_INET
                                 ? sizeof(struct sockaddr_in)
                                 : sizeof(struct sockaddr_in6);

  // Inicializa soquete do cliente
  int s;
  s = socket(storage.ss_family, SOCK_DGRAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  char client_option[BUFSZ];
  char response[BUFSZ];

  while (1) {
    printf("0 - Sair\n");
    printf("1 - O Senhor dos Anéis\n");
    printf("2 - O Poderoso Chefão\n");
    printf("3 - Clube da Luta\n");

    fgets(client_option, BUFSZ, stdin);

    // if (strncmp(client_option, "0", 1) == 0 ||
    //     strncmp(client_option, "1", 1) == 0 ||
    //     strncmp(client_option, "2", 1) == 0 ||
    //     strncmp(client_option, "3", 1) == 0) {
    //   break;
    // }

    if (strncmp(client_option, "0", 1) == 0) {
      close(s);
      exit(EXIT_SUCCESS);
      // break;
    }

    // Envia para o servidor opção escolhida pelo cliente
    size_t countSend = sendto(s, &client_option, sizeof(client_option), 0,
                              (struct sockaddr*)&storage, addr_storage_size);

    if (countSend < 0) {
      logexit("sendto");
    }

    for (int i = 0; i < 5; i++) {
      recvfrom(s, response, BUFSZ - 1, 0, NULL, NULL);
      printf("%s\n", response);
      memset(response, 0, BUFSZ);
    }
  }

  // if (strncmp(client_option, "0", 1) == 0) {
  //   close(s);
  //   exit(EXIT_SUCCESS);
  // }

  // // Envia para o servidor opção escolhida pelo cliente
  // size_t countSend = sendto(s, &client_option, sizeof(client_option), 0,
  //                           (struct sockaddr*)&storage, addr_storage_size);

  // if (countSend < 0) {
  //   logexit("sendto");
  // }

  // for (int i = 0; i < 5; i++) {
  //   recvfrom(s, response, BUFSZ - 1, 0, NULL, NULL);
  //   printf("%s\n", response);
  //   memset(response, 0, BUFSZ);
  // }

  // Fecha o socket do cliente
  close(s);

  return 0;
}