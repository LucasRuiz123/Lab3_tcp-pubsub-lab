#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_SIZE 1024

static const char *EVENTOS[] = {
    "Gol de Equipo A al minuto %d",
    "Tarjeta amarilla al numero 10 de Equipo B al minuto %d",
    "Cambio: jugador 10 entra por jugador 20 al minuto %d",
    "Gol de Equipo B al minuto %d",
    "Tiro de esquina para Equipo A al minuto %d"
};

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IOLBF, 0);
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <ip_broker> <puerto> <topic> [n_msgs] [delay_ms]\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port   = atoi(argv[2]);
    const char *topic = argv[3];
    int n_msgs = (argc > 4) ? atoi(argv[4]) : 10;
    int delay  = (argc > 5) ? atoi(argv[5]) : 500;

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
    printf("[PUBLISHER] Conectado: puerto_local=%d puerto_broker=%d\n", ntohs(local_addr.sin_port), port);
    fflush(stdout);

    for (int seq = 1; seq <= n_msgs; seq++) {
        char evento[256], msg[BUF_SIZE];
        snprintf(evento, sizeof(evento), EVENTOS[(seq - 1) % 5], seq * 4);
        snprintf(msg, sizeof(msg), "PUB %s %d %s", topic, seq, evento);

        if (send(sock, msg, strlen(msg), 0) < 0)
            perror("send");
        else
            printf("[PUB %s] enviado seq=%d: %s\n", topic, seq, evento);
        usleep(delay * 1000);
    }
    close(sock);
    return 0;
}