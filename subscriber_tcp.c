#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto> <topic>\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    const char *topic = argv[3];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in broker;
    memset(&broker, 0, sizeof(broker));
    broker.sin_family = AF_INET;
    broker.sin_port   = htons(port);
    if (inet_pton(AF_INET, ip, &broker.sin_addr) != 1) {
        fprintf(stderr, "IP invalida\n");
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&broker, sizeof(broker)) < 0) {
        perror("connect");
        return 1;
    }

    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &local_len);
    printf("[SUBSCRIBER] Conectado: puerto_local=%d puerto_broker=%d\n", ntohs(local_addr.sin_port), port);
    fflush(stdout);

    char sub_msg[BUF_SIZE];
    snprintf(sub_msg, sizeof(sub_msg), "SUB %s", topic);
    if (send(sock, sub_msg, strlen(sub_msg), 0) < 0) {
        perror("send");
        return 1;
    }
    printf("[SUB %s] Conectado al broker, esperando mensajes...\n", topic);

    char buffer[BUF_SIZE];
    while (1) {
        int n = recv(sock, buffer, BUF_SIZE - 1, 0);
        if (n <= 0) break;
        buffer[n] = '\0';
        printf("[SUB %s] Recibido: %s\n", topic, buffer);
    }

    close(sock);
    return 0;
}