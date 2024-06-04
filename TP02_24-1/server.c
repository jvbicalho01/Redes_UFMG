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

unsigned clients_connecteds = 0;

// estrutura que vai guardar os dados do cliente
typedef struct {
  char client_option[BUFSZ];
  int s_socket;
  struct sockaddr_storage storage;
} client_info;

// função responsável por realizar a lógica do programa com um cliente
void* client_handler(void* ptr) {
  client_info* data = (client_info*)ptr;
  char response[BUFSZ];

  struct sockaddr* caddr = (struct sockaddr*)&(data->storage);
  socklen_t caddrlen = sizeof(data->storage);

  // qual a opção escolhida pelo cliente e envias as respectivas frases
  if (strncmp(data->client_option, "1", 1) == 0) {
    for (int i = 0; i < 5; i++) {
      memset(response, 0, BUFSZ);
      strcpy(response, frases_senhorDosAneis[i]);
      // envia a mensagem para o cliente
      sendto(data->s_socket, response, BUFSZ - 1, 0, caddr, caddrlen);
      sleep(3);
    }

  } else if (strncmp(data->client_option, "2", 1) == 0) {
    for (int i = 0; i < 5; i++) {
      memset(response, 0, BUFSZ);
      strcpy(response, frases_poderosoChefao[i]);
      // envia a mensagem para o cliente
      sendto(data->s_socket, response, BUFSZ - 1, 0, caddr, caddrlen);
      sleep(3);
    }

  } else if (strncmp(data->client_option, "3", 1) == 0) {
    for (int i = 0; i < 5; i++) {
      memset(response, 0, BUFSZ);
      strcpy(response, frases_clubeDaLuta[i]);
      // envia a mensagem para o cliente
      sendto(data->s_socket, response, BUFSZ - 1, 0, caddr, caddrlen);
      sleep(3);
    }
  } else {
    // close(s);
    logexit("opção inválida!");
  }

  clients_connecteds--;

  pthread_exit(EXIT_SUCCESS);
}

// função responsavel por contar o número de clientes conectados
void* count_number_connections(void* ptr) {
  while (1) {
    printf("Clientes: %u\n", clients_connecteds);
    sleep(4);
  }

  pthread_exit(EXIT_SUCCESS);
}

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

  // cria a thread responsável por executar a função 'count_number_connections'
  pthread_t counter_clients;
  pthread_create(&counter_clients, NULL, count_number_connections, NULL);

  while (1) {
    // Endereço IP do cliente
    struct sockaddr_storage cstorage;
    struct sockaddr* caddr = (struct sockaddr*)(&cstorage);
    socklen_t caddrlen = sizeof(cstorage);

    char client_option[BUFSZ];
    // char response[BUFSZ];

    // recebe do cliente a opção escolhida
    size_t coutRecv =
        recvfrom(s, &client_option, sizeof(client_option), 0, caddr, &caddrlen);

    if (coutRecv < 0) {
      logexit("recvfrom");
    }

    client_info* cdata = malloc(sizeof(*cdata));
    if (cdata == NULL) {
      logexit("malloc");
    }

    // inicializa a estrutura com os dados referentes do cliente para passar
    // como parametro da função 'client_handler'
    strcpy(cdata->client_option, client_option);
    // cdata->client_option = client_option;
    cdata->s_socket = s;
    memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

    // cria a thread responsável por executar a função 'client_handler'
    pthread_t tid;
    pthread_create(&tid, NULL, client_handler, cdata);
    clients_connecteds++;
  }

  pthread_join(counter_clients, NULL);

  // fecha o socket do servidor
  close(s);

  return 0;
}