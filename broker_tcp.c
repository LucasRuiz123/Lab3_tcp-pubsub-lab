#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 5000
#define BUFFER_SIZE 1024
#define MAX_SUBSCRIBERS 10

typedef struct {
    int socket;
    char topic[256];
} Subscriber;

Subscriber subscribers[MAX_SUBSCRIBERS];
int subscriber_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *handle_client(void *arg) {
    int client_sock = (intptr_t)arg;
    char buffer[BUFFER_SIZE];
    char type[32], topic[256], rest[BUFFER_SIZE];

    int n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (n <= 0) {
        close(client_sock);
        return NULL;
    }
    buffer[n] = '\0';

    sscanf(buffer, "%s %s %[^\n]", type, topic, rest);

    if (strcmp(type, "SUB") == 0) {
        pthread_mutex_lock(&lock);
        if (subscriber_count < MAX_SUBSCRIBERS) {
            subscribers[subscriber_count].socket = client_sock;
            strcpy(subscribers[subscriber_count].topic, topic);
            printf("New subscriber for topic: %s\n", topic);
            subscriber_count++;
        }
        pthread_mutex_unlock(&lock);
        
        // Mantener socket abierto sin intentar recv()
        while (1) {
            sleep(1);
        }
    }
    else if (strcmp(type, "PUB") == 0) {
        printf("Message for topic %s: %s\n", topic, rest);
        
        pthread_mutex_lock(&lock);
        for (int i = 0; i < subscriber_count; i++) {
            if (strcmp(subscribers[i].topic, topic) == 0) {
                send(subscribers[i].socket, buffer, strlen(buffer), 0);
            }
        }
        pthread_mutex_unlock(&lock);
        
        // Recibir mas mensajes del publisher
        while (1) {
            n = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
            if (n <= 0) break;
            buffer[n] = '\0';
            
            sscanf(buffer, "%s %s %[^\n]", type, topic, rest);
            printf("Message for topic %s: %s\n", topic, rest);
            
            pthread_mutex_lock(&lock);
            for (int i = 0; i < subscriber_count; i++) {
                if (strcmp(subscribers[i].topic, topic) == 0) {
                    send(subscribers[i].socket, buffer, strlen(buffer), 0);
                }
            }
            pthread_mutex_unlock(&lock);
        }
        
        close(client_sock);
    }
    
    return NULL;
}

int main() {
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in broker_addr;
    memset(&broker_addr, 0, sizeof(broker_addr));
    broker_addr.sin_family = AF_INET;
    broker_addr.sin_port = htons(BROKER_PORT);
    inet_pton(AF_INET, BROKER_IP, &broker_addr.sin_addr);

    bind(listen_sock, (struct sockaddr *)&broker_addr, sizeof(broker_addr));
    listen(listen_sock, 5);
    printf("Broker listening on %s:%d\n", BROKER_IP, BROKER_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
        
        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, (void *)(intptr_t)client_sock);
    }

    close(listen_sock);
    return 0;
}