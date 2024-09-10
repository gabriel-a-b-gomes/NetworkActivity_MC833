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

int Socket(int family, int type, int flags) {
    int sockfd;

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
    if (inet_pton(AF_INET, addr, &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        exit(1);
    }
}

void GetSockName(int sockfd, struct sockaddr* servaddr, socklen_t servaddr_len) {
    if (getsockname(sockfd, servaddr, &servaddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void PrintSockName(char* description, struct sockaddr_in sockinfos, int family, int addr_len) {
    char p_addr[addr_len];
    inet_ntop(family, &(sockinfos.sin_addr), p_addr, addr_len);
    printf("%s (%s, %d)\n", description, p_addr, ntohs(sockinfos.sin_port));
}


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

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    Connect(sockfd, argv[1], atoi(argv[2]), AF_INET);

    GetSockName(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    PrintSockName("Socket local", servaddr, AF_INET, INET_ADDRSTRLEN);

    // fgets(sendline, MAXLINE, stdin);

    // if (write(sockfd, sendline, MAXLINE) < 0) {
    //     perror("write");
    //     exit(1);
    // }

    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0;

        printf("> %s\n", recvline);

        // if (fputs(recvline, stdout) == EOF) {
        //     perror("fputs error");
        //     exit(1);
        // }

        if (strcmp(recvline, "TAREFA: LIMPEZA") == 0) {
            sleep(5);

            snprintf(sendline, MAXLINE, "TAREFA_LIMPEZA CONCLUÍDA");

            printf("%s\n", sendline);

            if (write(sockfd, recvline, MAXLINE) < 0) {
                perror("write");
                exit(1);
            }

            printf("< TAREFA_LIMPEZA CONCLUÍDA");
        }
    }

    if (n < 0) {
        perror("read error");
        exit(1);
    }

    exit(0);
}
