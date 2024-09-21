#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXDATASIZE 2100
#define MAXLINE 2048

int Socket(int family, int type, int flags) {
    int sockfd;

    // Cria um novo descritor do socket
    if ((sockfd = socket(family, type, flags)) == -1) {
        perror("socket");
        exit(1);
    }

    return sockfd;
}

void Connect(int sockfd, char* addr, int port, int family) {
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = family;

    servaddr.sin_port   = htons(port);

    // Instancia o endereço passado por parametro
    if (inet_pton(AF_INET, addr, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    // Conecta-se com o servidor
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }
}

void GetSockName(int sockfd, struct sockaddr* servaddr, socklen_t servaddr_len) {
    // Obtem as informações do socket local
    if (getsockname(sockfd, servaddr, &servaddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void GetPeerName(int connfd, struct sockaddr* peeraddr, socklen_t peeraddr_len) {
    // Obtem as informações do peer do servidor
    if (getpeername(connfd, peeraddr, &peeraddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void PrintSockName(char* description, struct sockaddr_in sockinfos, int family, int addr_len) {
    char p_addr[addr_len];
    inet_ntop(family, &(sockinfos.sin_addr), p_addr, addr_len);
    // Printa as informações de IP e Porta do socket
    printf("%s (%s, %d)\n", description, p_addr, ntohs(sockinfos.sin_port));
}


int main(int argc, char **argv) {
    int    sockfd, n;
    char   recvline[MAXLINE + 1];
    char   sendline[MAXDATASIZE + 1];
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr;

    char prefixo[] = "tarefa";

    // Garante que o IP e Porta do socket servidor foram passados
    if (argc != 3) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>");
        strcat(error," <Port>");
        perror(error);
        exit(1);
    }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(sockfd, argv[1], atoi(argv[2]), AF_INET);

    // Obtem as informações do socket local e imprime o IP e Porta
    GetSockName(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Local", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Obtem as informações do socket servidor e imprime seu IP e Porta
    GetPeerName(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Servidor", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Aguarda por novas entradas vindas do servidor
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;

        // Mostra a mensagem recebida
        printf("> RECEBIDO: %s\n", recvline);

        // Verifica se a mensagem recebida inicia-se com a palavra "TAREFA"
        if (strncasecmp(recvline, prefixo, strlen(prefixo)) == 0) {
            // Aguarda por 5s
            sleep(5);

            memset(sendline, 0, sizeof(sendline));

            snprintf(sendline, MAXDATASIZE, "%s CONCLUÍDA", recvline);

            // Envia ao servidor que a tarefa recebida foi processada
            if (write(sockfd, sendline, MAXDATASIZE) < 0) {
                perror("write");
                exit(1);
            }

            printf("< ENVIADO: %s CONCLUÍDA\n", recvline);
        }

        // Trata a mensagem para encerrar a conexão
        if (strcmp(recvline, "ENCERRAR") == 0) {
            snprintf(sendline, MAXDATASIZE, "ENCERRAR\n");

            // Envia para o servidor que está encerrando a conexão
            if (write(sockfd, sendline, MAXDATASIZE) < 0) {
                perror("write");
                exit(1);
            }

            // Fecha a conexão
            close(sockfd);
            break;
        }
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}
