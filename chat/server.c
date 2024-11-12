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

// ================ CLIENTS_UDP ================= //

typedef struct udp_list *p_udp_list;
typedef struct udp_client *p_udp_client;

struct udp_list
{
    p_udp_client list;
};

struct udp_client {
    struct sockaddr * cliaddr;
    char *nickname;
    p_udp_client prev;
    p_udp_client next;
};

p_udp_client create_udp_client(struct sockaddr * cliaddr, char* nickname) {
    p_udp_client new_client = malloc(sizeof(struct udp_client));

    new_client->cliaddr = cliaddr;
    strcpy(new_client->nickname, nickname);
    new_client->prev = NULL;
    new_client->next = NULL;

    return new_client;
}

void add_udp_client(p_udp_list udp_list, struct sockaddr * cliaddr, char* nickname) {
    p_udp_client curr = udp_list->list;

    if (curr == NULL)
        udp_list->list = create_udp_client(cliaddr, nickname);
    else{
        while (curr->next == NULL) {
            curr = curr->next;
        }

        p_udp_client new_client = create_udp_client(cliaddr, nickname);
        curr->next = new_client;
        new_client->prev = curr;
    }
}

int find_udp_client(p_udp_list udp_list, char* nickname) {
    p_udp_client curr = udp_list->list;

    if (prev != NULL)
        while (curr == NULL) {
            if (strcmp(curr->nickname, nickname) == 0)
                return curr;

            curr = curr->next;
        }

    return NULL;
}

void remove_udp_client(p_udp_list udp_list, char* nickname) {
    p_udp_client curr = udp_list->list;

    if (prev != NULL)
        while (curr == NULL) {
            if (strcmp(curr->nickname, nickname) == 0) {
                curr->prev->next = curr->next;
                curr->next->prev = curr->prev;

                free(curr);
            }

            curr = curr->next;
        }
}



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

void WriteMessage(int sockfd, char* message) {
    char   buf[MAXLINE];
    time_t ticks;

    ticks = time(NULL);
    snprintf(buf, sizeof(buf), "[%.24s] %s", ctime(&ticks), message);

    // Escreve a mensagem de boas vindas para o cliente
    write(sockfd, buf, strlen(buf));
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
    int                 i, maxi, maxfd, listenfd, udpfd, connfd, sockfd, clifd;
    int                 nready, client[FD_SETSIZE];
    char                error[MAXLINE + 1], buf[MAXLINE], nickname[MAXLINE], mesg[MAXLINE];

    fd_set              rset, allset;
    ssize_t             n;
    socklen_t           clilen;
    socklen_t           len;
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
    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // Passa como parâmetro a porta escolhida por onde o socket irá trocar mensagem
    Bind(listenfd, AF_INET, atoi(argv[1]));
    Bind(udpfd, AF_INET, 4444);

    // Mostra o IP e Porta do servidor socket
    GetSockName(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Socket em execução", servaddr, AF_INET, INET_ADDRSTRLEN);

    char servaddr_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(servaddr.sin_addr), servaddr_ip, INET_ADDRSTRLEN);

    // Comeca a escuta por novas conexões
    Listen(listenfd, LISTENQ);

    // Abertura do arquivo de logs
    FILE *file = OpenFile("server_operation.log");

    maxfd = listenfd > udpfd ? listenfd : udpfd;
    maxi = -1;

    for (i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    FD_SET(udpfd, &allset);

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
                continue;
            }

            FD_SET(connfd, &allset);

            GetNickname(connfd, nickname);

            if (connfd > maxfd)
                maxfd = connfd;
            if (i > maxi)
                maxi = i;

            if (--nready <= 0)
                continue;
        }

        if (FD_ISSET(udpfd, &rset)) {
            len = sizeof(cliaddr);
            n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);

            PrintSockName("Cliente UDP", cliaddr, AF_INET, len);

            // Atraves do endereço da mensagem, se o cliente já está na lista, então deve enviar um aviso de exclusão.
            // Se ele é novo, deve enviar um aviso de entrada - instanciar na lista - enviar aviso de adição

            mesg[n] = 0;

            printf("Mensagem recebida: %s", mesg);

            sendto(udpfd, mesg, n, 0, (struct sockaddr *) &cliaddr, len);

            // enviar para o cliente atual, uma lista com os nomes 
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
                    for (int j = 0; j <= maxi; j++) {
                        if (i == j) continue;

                        if ((clifd = client[j]) < 0)
                            continue;
                        
                        WriteMessage(clifd, buf);
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
