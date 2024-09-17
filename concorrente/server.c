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


FILE * OpenFile(char * fileName) {
    FILE *file = fopen(fileName, "w");

    if (file == NULL) {
        perror("Arquivo not found");
        exit(1);
    }

    return file;
}

void WriteFile(FILE * file, char * message, char * addr, int port) {
    fprintf(file, "%s | Cliente (%s, %d)\n", message, addr, port);
}

void CloseFile(FILE * file) {
    fclose(file);
}


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
    servaddr.sin_port        = htons(port);   

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

    sleep(1);
}


char * GetTask(int taskid) {
    switch (taskid)
    {
        case 1: 
            return "TAREFA:LIMPEZA";
        case 2: 
            return "TAREFA:FORMATACAO";
        case 3: 
            return "TAREFA:RESTAURACAO";
        case 4: 
            return "TAREFA:PEDIDO";
        case 5: 
            return "TAREFA:REPORTAR";
        default:
            return "TAREFA:NOP";
    }

    return "TAREFA:NOP";
}


void ProcessTasks(int connfd, int taskid, FILE *file, char* addr, int port) {
    int n;
    char buf[MAXDATASIZE];
    char recvline[MAXLINE + 1];
    char template[MAXLINE + 1];

    char* task = GetTask(taskid);

    snprintf(template, MAXLINE, "%s CONCLUÍDA", task);

    snprintf(buf, sizeof(buf), "%s", task);

    write(connfd, buf, strlen(buf));

    printf("< ENVIADO: %s\n", task);

    WriteFile(file, buf, addr, port);

    while ( (n = read(connfd, recvline, sizeof(recvline))) > 0) {
        recvline[n] = 0;

        printf("> RECEBIDO: %s\n", recvline);
        WriteFile(file, recvline, addr, port);

        if (strcmp(recvline, template) == 0)
            break;
    }
}

void SendCloseMessage(int connfd) {
    char   buf[MAXDATASIZE];

    snprintf(buf, sizeof(buf), "ENCERRAR");

    write(connfd, buf, strlen(buf));
}

void ReceiveCloseMessage(int connfd) {
    int n;
    char recvline[MAXLINE + 1];

    while ( (n = read(connfd, recvline, sizeof(recvline))) > 0) {
        recvline[n] = 0;

        if (strcmp(recvline, "ENCERRAR") == 0) {
            break;
        }
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
    char   error[MAXLINE + 1];
    pid_t pid;

    if (argc != 2) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <Port>");
        perror(error);
        exit(1);
    }

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    Bind(listenfd, AF_INET, atoi(argv[1]));

    struct sockaddr_in servaddr;

    GetSockName(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));

    PrintSockName("Socket em execução", servaddr, AF_INET, INET_ADDRSTRLEN);

    Listen(listenfd, LISTENQ);

    FILE *file = OpenFile("server_operation.log");

    for ( ; ; ) {
        struct sockaddr_in peeraddr;

        connfd = Accept(listenfd);

        if ((pid = Fork()) == 0) 
        {
            Close(listenfd);
            
            int min_tasks = 1;
            int max_tasks = 5;
            int num_tasks = min_tasks + rand() % (max_tasks - min_tasks + 1);

            GetPeerName(connfd, (struct sockaddr*)&peeraddr, sizeof(peeraddr));

            char paddrClient[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(peeraddr.sin_addr), paddrClient, INET_ADDRSTRLEN);

            int portClient = ntohs(peeraddr.sin_port);

            PrintSockName("Conexão Recebida", peeraddr, AF_INET, INET_ADDRSTRLEN);

            WriteWelcomeMessage(connfd);

            int i = 1;
            while (i <= num_tasks) { 
                ProcessTasks(connfd, i, file, paddrClient, portClient);
                i++;
            }

            SendCloseMessage(connfd);

            Close(connfd);

            exit(0);
        }

        Close(connfd);
    }

    CloseFile(file);
    return(0);
}
