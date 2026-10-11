#ifndef CONFIG_H
#define CONFIG_H

// ===== BROKER CONFIGURATION =====
#define BROKER_IP "127.0.0.1"
#define BROKER_PORT 5000

// ===== MESSAGE BUFFER SIZE =====
#define BUFFER_SIZE 2048  // ← Cambio de 256 a 512

// ===== MAX CONNECTIONS =====
#define MAX_PUBLISHERS 10
#define MAX_SUBSCRIBERS 10
#define MAX_TOPICS 20

#endif