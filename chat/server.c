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

#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

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

int Accept(int sockfd, struct sockaddr * cliaddr, socklen_t * clilen) {
    int connfd;

    // Aceita uma nova conexão
    if ((connfd = accept(sockfd, cliaddr, clilen)) == -1 ) {
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


char * GetTask(int taskid) {
    // Seleção de uma task a partir da task id
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

    // Obtem a task que será processada
    char* task = GetTask(taskid);

    // Template de conclusao da task - nome_task CONCLUIDA
    snprintf(template, MAXLINE, "%s CONCLUÍDA", task);

    snprintf(buf, sizeof(buf), "%s", task);

    // Envia ao cliente a task para processamento
    write(connfd, buf, strlen(buf));

    // Escreve no arquivo de logs que foi enviada uma nova tarefa
    WriteFile(file, "ENVIADO", buf, addr, port);

    // Aguarda pela leitura de uma nova mensagem
    while ( (n = read(connfd, recvline, sizeof(recvline))) > 0) {
        recvline[n] = 0;

        // Escreve nos logs a mensagem recebida do cliente
        WriteFile(file, "RECEBIDO", recvline, addr, port);

        // Verifica se a mensagem é igual ao template definido
        if (strcmp(recvline, template) == 0)
            break;
    }
}

void SendCloseMessage(int connfd, FILE *file, char* addr, int port) {
    char   buf[MAXDATASIZE];

    snprintf(buf, sizeof(buf), "ENCERRAR");
    WriteFile(file, "ENVIADO", buf, addr, port);

    // Envia uma mensagem ao cliente para encerrar
    write(connfd, buf, strlen(buf));
}

void Close(int sockfd) {
    close(sockfd);
}

void Monitoring(int connfd, const char *ip, int port) {
    char   buffer[MAXLINE];

    time_t t = time(NULL);

    srand(t);

    struct tm *tm_info = localtime(&t);
    char horario[26];
    strftime(horario, 26, "%c", tm_info);

    int cpu = rand() % 101;
    int memoria = rand() % 101;
    const char *status = (cpu % 2 == 0) ? "Ativo" : "Inativo";

    snprintf(buffer, MAXLINE,
             "-------------------\nMonitoramento do servidor:\nIP: %s\nPorta: %d\nHorário: %s\nCPU: %d%%\nMemória: %d%%\nStatus: %s\n-------------------",
             ip, port, horario, cpu, memoria, status);

    write(connfd, buffer, MAXLINE);
}

char *GetNickname(int connfd, char* nickname) {
    int n;

    while((n = read(connfd, nickname, sizeof(nickname))) > 0) {
        nickname[n] = 0;

        break;
    }

    return nickname;
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

int main (int argc, char **argv) {
    int                 i, maxi, maxfd, listenfd, connfd, sockfd, clifd;
    int                 nready, client[FD_SETSIZE];
    char                error[MAXLINE + 1], buf[MAXLINE], nickname[MAXLINE];

    fd_set              rset, allset;
    ssize_t             n;
    socklen_t           clilen;
    struct sockaddr_in  cliaddr, servaddr;

    // Verifica se a porta foi passada como parâmetro
    if (argc != 2) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <Port>");
        perror(error);
        exit(1);
    }

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    // Passa como parâmetro a porta escolhida por onde o socket irá trocar mensagem
    Bind(listenfd, AF_INET, atoi(argv[1]));

    // Mostra o IP e Porta do servidor socket
    GetSockName(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket em execução", servaddr, AF_INET, INET_ADDRSTRLEN);

    char servaddr_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), servaddr_ip, INET_ADDRSTRLEN);

    // Comeca a escuta por novas conexões
    Listen(listenfd, LISTENQ);

    // Abertura do arquivo de logs
    FILE *file = OpenFile("server_operation.log");

    maxfd = listenfd;
    maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    for ( ; ; ) {
        rset = allset;

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                perror("Clientes demais");
                exit(1);
            }

            FD_SET(connfd, &allset);

            GetNickname(connfd, nickname);

            printf("[%s] Conectou-se", nickname);

            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        for (i = 0; i <= maxi; i++) {
            if ((sockfd = client[i]) < 0)
                continue;

            if (FD_ISSET(sockfd, &rset)) {
                if ((n = read(sockfd, buf, MAXLINE)) == 0) {
                    Close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    for (int j = 0; j < maxi; j++) {
                        if (i == j) continue;

                        if ((clifd = client[j]) < 0)
                            continue;
                        
                        write(clifd, buf, n);
                    }
                    // write(sockfd, buf, n);
                }

                if (--nready <= 0)
                    break;
            }
        }
    }

    CloseFile(file);
    return(0);
}
