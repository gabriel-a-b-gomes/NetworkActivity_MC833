#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv) {
    int    listenfd, connfd;
    struct sockaddr_in servaddr;
    char   buf[MAXDATASIZE];
    char   recvline[MAXLINE + 1];
    time_t ticks;


    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(0);   

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    socklen_t servaddr_len = sizeof(servaddr);
    if (getsockname(listenfd, (struct sockaddr*)&servaddr, &servaddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }

    printf("Socket rodando na porta: %d\n", ntohs(servaddr.sin_port));

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen");
        exit(1);
    }

    for ( ; ; ) {
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
        }

        struct sockaddr_in peeraddr;
        socklen_t peeraddr_len = sizeof(peeraddr);
        if (getpeername(connfd, (struct sockaddr*)&peeraddr, &peeraddr_len) == -1) {
            perror("getsockname");
            exit(1);
        }

        char p_addr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(peeraddr.sin_addr), p_addr, INET_ADDRSTRLEN);
        printf("Conexão recebida (%s, %d)\n", p_addr, ntohs(peeraddr.sin_port));

        int n = read(connfd, recvline, MAXLINE);
        recvline[n] = 0;

        printf("Mensagem recebida: %s\n", recvline);

        ticks = time(NULL);
        snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));
        write(connfd, buf, strlen(buf));


        close(connfd);
    }
    return(0);
}
