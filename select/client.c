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

int max(int a, int b) {
    return a >= b ? a : b;
}

// ==================== FILE ==================== //

FILE * OpenFile(char * fileName, char * mode) {
    // Abre o descritor do arquivo baseado no filename
    FILE *file = fopen(fileName, mode);

    if (file == NULL) {
        perror("Arquivo não encontrado");
        exit(1);
    }

    return file;
}

void WriteFile(FILE * file, char * message, int n) {
    // Escreve no arquivo um novo log, com o tempo, cliente e mensagem
    message[n] = 0;

    // printf("%s", message);
    
    if (n > 0 && message[n - 1] != '\n')
        fprintf(file, "%s\n", message);
    else
        fprintf(file, "%s", message);
    fflush(file);
}

void CloseFile(FILE * file) {
    fclose(file);
}

// ==================== FILE ==================== //

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

void str_cli(FILE * fp, FILE * exitfp, int sockfd1, int sockfd2) {
    int maxfdp, stdineof;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;

    stdineof = 0;
    FD_ZERO(&rset);

    for (;;) {
        if (stdineof == 0) 
            FD_SET(fileno(fp), &rset);

        FD_SET(sockfd1, &rset);
        FD_SET(sockfd2, &rset);
        maxfdp = max(fileno(fp), (sockfd1 > sockfd2 ? sockfd1 : sockfd2)) + 1;
        
        select(maxfdp, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd1, &rset)) {
            if ((n = read(sockfd1, recvline, MAXLINE)) == 0) {
                if (stdineof == 1)
                    return;
                else {
                    perror("str_cli: servidor finalizou antes do esperado");
                    exit(1);
                }
            }

            WriteFile(exitfp, recvline, n);
        }

        if (FD_ISSET(sockfd2, &rset)) {
            if ((n = read(sockfd2, recvline, MAXLINE)) == 0) {
                if (stdineof == 1)
                    return;
                else {
                    perror("str_cli: servidor finalizou antes do esperado");
                    exit(1);
                }
            }

            WriteFile(exitfp, recvline, n);
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                stdineof = 1;
                shutdown(sockfd1, SHUT_WR);
                shutdown(sockfd2, SHUT_WR);
                continue;
            }

            write(sockfd1, sendline, strlen(sendline));
            write(sockfd2, sendline, strlen(sendline));
        }
    }
}

int main(int argc, char **argv) {
    int    sockfd1, sockfd2;
    char   error[MAXLINE + 1];
    struct sockaddr_in servaddr;

    // Garante que o IP e Porta do socket servidor foram passados
    if (argc != 6) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>");
        strcat(error," <Port1>");
        strcat(error," <Port2>");
        strcat(error," <entrada.txt>");
        strcat(error," <saida.txt>");
        perror(error);
        exit(1);
    }

    sockfd1 = Socket(AF_INET, SOCK_STREAM, 0);

    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(sockfd1, argv[1], atoi(argv[2]), AF_INET);

    // Obtem as informações do socket local e imprime o IP e Porta
    GetSockName(sockfd1, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Local 1", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Obtem as informações do socket servidor e imprime seu IP e Porta
    GetPeerName(sockfd1, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Servidor 1", servaddr, AF_INET, INET_ADDRSTRLEN);

    sockfd2 = Socket(AF_INET, SOCK_STREAM, 0);

    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(sockfd2, argv[1], atoi(argv[3]), AF_INET);

    // Obtem as informações do socket local e imprime o IP e Porta
    GetSockName(sockfd2, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Local 2", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Obtem as informações do socket servidor e imprime seu IP e Porta
    GetPeerName(sockfd2, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket Servidor 2", servaddr, AF_INET, INET_ADDRSTRLEN);

    FILE * entryfp = OpenFile(argv[4], "r");
    FILE * exitfp = OpenFile(argv[5], "w");

    str_cli(entryfp, exitfp, sockfd1, sockfd2);

    CloseFile(entryfp);
    CloseFile(exitfp);

    shutdown(sockfd1, SHUT_WR);
    shutdown(sockfd2, SHUT_WR);

    close(sockfd1);
    close(sockfd2);

    exit(0);
}
