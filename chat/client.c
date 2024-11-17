#include <ctype.h>
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
#include <sys/wait.h>

#define MAXDATASIZE 2100
#define MAXLINE 2048
#define MAXNICK 1024

// Var para receber o nickname
char   nickname[MAXLINE + 1];
char   addr[MAXLINE + 1];

int max(int a, int b) {
    return a >= b ? a : b;
}


// Function to trim leading and trailing '|' characters from a string.
void trim_pipes(char *str) {
    if (str == NULL || strlen(str) == 0) return;

    // Trim leading '|'.
    while (*str == '|') str++;

    // Find the end of the string.
    char *end = str + strlen(str) - 1;

    // Trim trailing '|'.
    while (end > str && *end == '|') {
        *end = '\0';
        end--;
    }
}

// ================ CLIENTS_UDP ================= //

typedef struct people_list *p_people_list;
typedef struct people *p_people;

struct people_list
{
    p_people list;
};

struct people {
    char nickname[MAXNICK];
    p_people prev;
    p_people next;
};

p_people_list create_people_list() {
    p_people_list pop_list = malloc(sizeof(struct people_list));

    pop_list->list = NULL;

    return pop_list;
}

p_people create_people(char* nickname) {
    p_people new_people = malloc(sizeof(struct people));

    strcpy(new_people->nickname, nickname);
    new_people->prev = NULL;
    new_people->next = NULL;

    return new_people;
}

void add_people(p_people_list pop_list, char* nickname) {
    if (pop_list == NULL) {
        fprintf(stderr, "List is uninitialized or empty\n");
        return;
    }

    p_people curr = pop_list->list;

    if (curr == NULL)
        pop_list->list = create_people(nickname);
    else{
        while (curr->next != NULL) {
            curr = curr->next;
        }

        p_people new_people = create_people(nickname);
        curr->next = new_people;
        new_people->prev = curr;
    }

    
}

p_people find_people(p_people_list pop_list, char* nickname) {
    if (pop_list == NULL || pop_list->list == NULL) {
        fprintf(stderr, "List is uninitialized or empty\n");
        return 0;
    }

    p_people curr = pop_list->list;

    while (curr != NULL) {
        if (strcmp(curr->nickname, nickname) == 0)
            return curr;

        curr = curr->next;
    }

    return NULL;
}

int remove_people(p_people_list pop_list, char* nickname) {
    if (pop_list == NULL || pop_list->list == NULL) {
        fprintf(stderr, "List is uninitialized or empty\n");
        return 0;
    }

    p_people curr = pop_list->list;

    while (curr != NULL) {
        if (strcmp(curr->nickname, nickname) == 0) {
            if (curr->prev != NULL)
                curr->prev->next = curr->next;
            else
                pop_list->list = curr->next;
            
            if (curr->next != NULL)
                curr->next->prev = curr->prev;

            free(curr);
            curr = NULL;

            return 1;
        }

        curr = curr->next;
    }

    return 0;
}

void handle_udp_message(p_people_list pop_list, char *message) {
    // Check if the message is valid.
    if (message == NULL || strlen(message) < 2) {
        fprintf(stderr, "Invalid message received\n");
        return;
    }

    char *token;
    // Get the command character.
    char command = message[0];
    char *payload = message + 1; // Skip the command character.

    trim_pipes(payload);

    switch (command) {
        case 'A':
            token = strtok(payload, "|");

            // Add a nickname to the list.
            add_people(pop_list, token); // Example IP and port used.
            printf("Entrou no chat > %s\n", token);
            break;
        
        case 'R':
            token = strtok(payload, "|");

            // Remove a nickname from the list.
            if (remove_people(pop_list, token)) {
                printf("Saiu do chat < %s\n", token);
            } 
            break;

        case 'L': {
            // Replace the list with nicknames from the payload.
            // Tokenize the payload to separate nicknames.
            int qtde_people = 0;
            token = strtok(payload, "|");

            while (token != NULL) {
                trim_pipes(token);
                add_people(pop_list, token); // Example IP and port used.

                if (strcmp(token, nickname) != 0) {
                    printf("No chat > %s\n", token);
                    qtde_people++;
                }

                token = strtok(NULL, "|");

            }

            if (qtde_people == 0)
                printf("Você é o único no chat!\n");

            break;
        }    

        default:
            break;
    }
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

void make_chat(FILE * fp, int sockfd, int udpfd, p_people_list pop_list) {
    int maxfdp, stdineof;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;

    stdineof = 0;
    FD_ZERO(&rset);

    for (;;) {
        if (stdineof == 0) 
            FD_SET(fileno(fp), &rset);

        FD_SET(sockfd, &rset);
        FD_SET(udpfd, &rset);
        maxfdp = max(fileno(fp), sockfd > udpfd ? sockfd : udpfd) + 1;
        
        select(maxfdp, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {
            recvline[0] = 0;
            if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
                if (stdineof == 1)
                    return;
                else {
                    perror("str_cli: servidor finalizou antes do esperado");
                    exit(1);
                }
            }

            recvline[n] = 0;

            WriteFile(stdout, recvline, n);
        }

        if (FD_ISSET(udpfd, &rset)) {
            recvline[0] = 0;
            if ((n = recvfrom(udpfd, recvline, MAXLINE, 0, NULL, NULL)) == 0) {
                if (stdineof == 1)
                    return;
                else {
                    perror("str_cli: servidor finalizou antes do esperado");
                    exit(1);
                }
            }

            recvline[n] = 0;

            handle_udp_message(pop_list, recvline);
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            if (fgets(sendline, MAXLINE, fp) == NULL) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                continue;
            }

            write(sockfd, sendline, strlen(sendline));
        }
    }
}

void handle_exit_signal(int signum) {
    int udpfd;
    struct sockaddr_in udpaddr;

    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(udpfd, addr, 4444, AF_INET);

    GetPeerName(udpfd, (struct sockaddr*) &udpaddr, sizeof(udpaddr));

    if (strlen(nickname) > 0) {
        sendto(udpfd, nickname, strlen(nickname), 0, (struct sockaddr *) &udpaddr, sizeof(udpaddr));
    }
    // Close the UDP socket and perform any other cleanup
    close(udpfd);
    exit(0);
}

int main(int argc, char **argv) {
    signal(SIGINT, handle_exit_signal);

    int    sockfd, udpfd, n;
    char   error[MAXLINE + 1], recvline[MAXLINE];
    struct sockaddr_in servaddr, udpaddr;

    p_people_list pop_list = create_people_list();

    // Garante que o IP e Porta do socket servidor foram passados
    if (argc != 3) {
        strcpy(error,"uso: ");
        strcat(error,argv[0]);
        strcat(error," <IPaddress>");
        strcat(error," <Port>");
        perror(error);
        exit(1);
    }

    printf("Insira seu nome: ");
    fgets(nickname, MAXLINE, stdin);

    n = strlen(nickname);

    // Tira o \n
    if (n > 0)
        nickname[n - 1] = 0;

    // size_t len = strlen(nickname);
    // if (len > 0 && (nickname[len - 1] == '\n' || nickname[len - 1] == '\r')) {
    //     nickname[len - 1] = '\0';
    // }

    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    strcpy(addr, argv[1]);
    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(sockfd, addr, atoi(argv[2]), AF_INET);

    // Obtem as informações do socket local e imprime o IP e Porta
    GetSockName(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Bem vindo ao chat!", servaddr, AF_INET, INET_ADDRSTRLEN);

    // Obtem as informações do socket servidor e imprime seu IP e Porta
    GetPeerName(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    PrintSockName("Endereço:", servaddr, AF_INET, INET_ADDRSTRLEN);

    write(sockfd, nickname, strlen(nickname));

    n = read(sockfd, recvline, MAXLINE);
    recvline[n] = 0;
    if (strcmp(recvline, "R") == 0) {
        close(sockfd);
        printf("AVISO: Já existe uma pessoa com esse nome no chat!\n");

        exit(0);
    }

    udpfd = Socket(AF_INET, SOCK_DGRAM, 0);

    // Conecta-se com o socket servidor a partir dos parâmetros de entrada
    Connect(udpfd, addr, 4444, AF_INET);

    // Obtem as informações do socket local e imprime o IP e Porta
    GetSockName(udpfd, (struct sockaddr*) &udpaddr, sizeof(udpaddr));
    PrintSockName("Server UDP", udpaddr, AF_INET, INET_ADDRSTRLEN);

    // Obtem as informações do socket servidor e imprime seu IP e Porta
    GetPeerName(udpfd, (struct sockaddr*) &udpaddr, sizeof(udpaddr));
    PrintSockName("Server UDP", udpaddr, AF_INET, INET_ADDRSTRLEN);

    sendto(udpfd, nickname, strlen(nickname), 0, (struct sockaddr *) &udpaddr, sizeof(udpaddr));

    make_chat(stdin, sockfd, udpfd, pop_list);

    shutdown(sockfd, SHUT_WR);
    shutdown(udpfd, SHUT_WR);
    // shutdown(sockfd2, SHUT_WR);

    close(sockfd);
    close(udpfd);
    // close(sockfd2);

    exit(0);
}
