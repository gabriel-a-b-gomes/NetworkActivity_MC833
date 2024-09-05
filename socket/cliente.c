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

#define MAXLINE 4096

int main(int argc, char **argv) {
    int    sockfd, n;
    char   recvline[MAXLINE + 1];
    char   sendline[MAXLINE + 1];
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr;

    if (argc != 3) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>");
        strcat(error," <Port>");
        perror(error);
        exit(1);
    }

    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    // Recebe como parametro a porta do socket
    servaddr.sin_port   = htons(atoi(argv[2]));
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }

    // Obtem o IP e Porta do socket (Ex.: 5)
    socklen_t servaddr_len = sizeof(servaddr);
    if (getsockname(sockfd, (struct sockaddr*)&servaddr, &servaddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }

    // Print do IP e Porta do socket local (Ex.: 5)
    char s_addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), s_addr, INET_ADDRSTRLEN);
    printf("Socket IP: %s\n", s_addr);
    printf("Socket porta: %d\n", ntohs(servaddr.sin_port));

    // Recebe a mensagem que foi escrita na entrada STDIN
    fgets(sendline, MAXLINE, stdin);

    // Envia ao servidor a mensagem escrita (Ex.: 7)
    if (write(sockfd, sendline, MAXLINE) < 0) {
        perror("write");
        exit(1);
    }

    // Recebe as mensagem que o servidor enviou
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;
        if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(1);
        }
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}
