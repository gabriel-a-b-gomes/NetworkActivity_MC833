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

// ==================== FILE ==================== //

FILE * OpenFile(char * fileName) {
    // Abre o descritor do arquivo baseado no filename
    FILE *file = fopen(fileName, "w");

    if (file == NULL) {
        perror("Arquivo não encontrado");
        exit(1);
    }

    return file;
}

void WriteFile(FILE * file, char * type, char * message, char * addr, int port) {
    time_t ticks;

    ticks = time(NULL);

    // Retira o \n da mensagem de tempo
    char *time_str = ctime(&ticks);
    time_str[strlen(time_str) - 1] = '\0';

    // Escreve no arquivo um novo log, com o tempo, cliente e mensagem
    fprintf(file, "[%s] Cliente (%s, %d) | <%s> %s\n", time_str, addr, port, type, message);
    fflush(file);
}

void CloseFile(FILE * file) {
    fclose(file);
}

// ==================== FILE ==================== //

// =================== SOCKET =================== //

int Socket(int family, int type, int flags) {
    int sockfd;

    // Instancia um descritor do socket
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
    servaddr.sin_port        = htons(port);   

    // Binda a configuração do socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
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

void Listen(int sockfd, int listenQ) {
    // Inicia a escuta por novas conexões
    if (listen(sockfd, listenQ) == -1) {
        perror("listen");
        exit(1);
    }
}

int Accept(int sockfd) {
    int connfd;

    // Aceita uma nova conexão
    if ((connfd = accept(sockfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        perror("accept");
        exit(1);
    }

    return connfd;
}

void GetPeerName(int connfd, struct sockaddr* peeraddr, socklen_t peeraddr_len) {
    // Obtem as informações do peer que conectou-se
    if (getpeername(connfd, peeraddr, &peeraddr_len) == -1) {
        perror("getsockname");
        exit(1);
    }
}

void PrintSockName(char* description, struct sockaddr_in sockinfos, int family, int addr_len) {
    char p_addr[addr_len];
    inet_ntop(family, &(sockinfos.sin_addr), p_addr, addr_len);

    // Printa no terminal o IP e Porta do socket
    printf("%s (%s, %d)\n", description, p_addr, ntohs(sockinfos.sin_port));
}

void WriteWelcomeMessage(int connfd) {
    char   buf[MAXDATASIZE];
    time_t ticks;

    ticks = time(NULL);
    snprintf(buf, sizeof(buf), "Hello from server!\nTime: %.24s\r\n", ctime(&ticks));

    // Escreve a mensagem de boas vindas para o cliente
    write(connfd, buf, strlen(buf));

    sleep(1);
}


void Echo(int connfd) {
    int n;
    char recvline[MAXLINE + 1];

    while ((n = read(connfd, recvline, sizeof(recvline))) > 0) {
        recvline[n] = 0;

        write(connfd, recvline, strlen(recvline));
    }
}


void Close(int sockfd) {
    close(sockfd);
}

// =================== SOCKET =================== //

// ==================== FORK ==================== //

int Fork() {
    pid_t pid;

    // Cria um processo filho
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    return pid;
}

// ==================== FORK ==================== //

// =================== SIGNAL =================== //

typedef void Sigfunc(int);

Sigfunc * Signal(int signo, SigFunc * func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (signo == SIGALRM) {
#ifdef
    }
}

// =================== SIGNAL =================== //

int main (int argc, char **argv) {
    int    listenfd, connfd;
    char   error[MAXLINE + 1];
    pid_t pid;

    // Verifica se a porta foi passada como parâmetro
    if (argc != 3) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <Port>");
        strcat(error," <Backlog>");
        perror(error);
        exit(1);
    }

    int port = atoi(argv[1]);
    int backlog = atoi(argv[2]);

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Passa como parâmetro a porta escolhida por onde o socket irá trocar mensagem
    Bind(listenfd, AF_INET, port);

    struct sockaddr_in servaddr;

    // Mostra o IP e Porta do servidor socket
    GetSockName(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket em execução", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Comeca a escuta por novas conexões
    Listen(listenfd, backlog);

    // Abertura do arquivo de logs
    // FILE *file = OpenFile("server_operation.log");

    for ( ; ; ) {
        struct sockaddr_in peeraddr;

        connfd = Accept(listenfd);

        // Quando uma nova conexao é aceita, o servidor cria um novo processo 
        // que é responsavel por trocar mensagens com o cliente 
        if ((pid = Fork()) == 0) 
        {
            Close(listenfd);

            // Obtem-se o IP e Porta do cliente para uso posteriores
            // Esse será usado para identificar o cliente nos logs
            GetPeerName(connfd, (struct sockaddr*)&peeraddr, sizeof(peeraddr));
            PrintSockName("Conexão Recebida", peeraddr, AF_INET, INET_ADDRSTRLEN);

            Echo(connfd);

            // Fecha-se a conexão com o cliente
            Close(connfd);

            exit(0);
        }

        Close(connfd);
    }

    return(0);
}
