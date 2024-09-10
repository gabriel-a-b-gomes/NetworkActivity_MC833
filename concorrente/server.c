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

int Socket(int family, int type, int flags) {
    int sockfd;

    if ((sockfd = socket(family, type, flags)) == -1) {
        perror("socket");
        exit(1);
    }

    return sockfd;
}

void Bind(int sockfd, int family, int port) {
    struct sockaddr_in servaddr;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = family;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(0);   

    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        exit(1);
    }
}

void GetSockName(int sockfd, struct sockaddr* servaddr, socklen_t servaddr_len) {
    if (getsockname(sockfd, servaddr, &servaddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void Listen(int sockfd, int listenQ) {
    if (listen(sockfd, listenQ) == -1) {
        perror("listen");
        exit(1);
    }
}

int Accept(int sockfd) {
    int connfd;

    if ((connfd = accept(sockfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
    }

    return connfd;
}

void GetPeerName(int connfd, struct sockaddr* peeraddr, socklen_t peeraddr_len) {
    if (getpeername(connfd, peeraddr, &peeraddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void PrintSockName(char* description, struct sockaddr_in sockinfos, int family, int addr_len) {
    char p_addr[addr_len];
    inet_ntop(family, &(sockinfos.sin_addr), p_addr, addr_len);
    printf("%s (%s, %d)\n", description, p_addr, ntohs(sockinfos.sin_port));
}

void WriteWelcomeMessage(int connfd) {
    char   buf[MAXDATASIZE];
    time_t ticks;

    ticks = time(NULL);
    snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));

    write(connfd, buf, strlen(buf));
}

void ProcessTasks(int connfd, char* recvline) {
    int n;
    char buf[MAXDATASIZE];

    snprintf(buf, sizeof(buf), "TAREFA: LIMPEZA");

    write(connfd, buf, strlen(buf));

    while ( (n = read(connfd, recvline, sizeof(recvline))) > 0) {
        recvline[n] = 0;

        printf("> %s\n", recvline);

        // if (fputs(recvline, stdout) == EOF) {
        //     perror("fputs error");
        //     exit(1);
        // }

        if (strcmp(recvline, "TAREFA_LIMPEZA CONCLUÍDA") == 0)
            break;
    }
}

void Close(int sockfd) {
    close(sockfd);
}

int Fork() {
    pid_t pid;

    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    return pid;
}

int main (int argc, char **argv) {
    int    listenfd, connfd;
    
    char   recvline[MAXLINE + 1];
    pid_t pid;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    Bind(listenfd, AF_INET, 0);

    struct sockaddr_in servaddr;

    GetSockName(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    PrintSockName("Socket em execução", servaddr, AF_INET, INET_ADDRSTRLEN);

    Listen(listenfd, LISTENQ);

    for ( ; ; ) {
        struct sockaddr_in peeraddr;

        connfd = Accept(listenfd);

        if ((pid = Fork()) == 0) 
        {
            Close(listenfd);

            GetPeerName(connfd, (struct sockaddr*)&peeraddr, sizeof(peeraddr));

            PrintSockName("Conexão Recebida", peeraddr, AF_INET, INET_ADDRSTRLEN);

            WriteWelcomeMessage(connfd);

            ProcessTasks(connfd, recvline);

            Close(connfd);

            exit(0);
        }

        Close(connfd);
    }
    return(0);
}
