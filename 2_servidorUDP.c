#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/* Servidor UDP de mayúsculas.
   Escucha en el puerto indicado y para cada datagrama recibido convierte
   su contenido a mayúsculas y lo envía de vuelta al remitente.
*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return 1;
    }

    int puerto = atoi(argv[1]);
    if (puerto <= 0 || puerto > 65535) {
        fprintf(stderr, "Puerto inválido\n");
        return 1;
    }

    /* Crear socket UDP y bind a todas las interfaces en el puerto */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(puerto);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    printf("Servidor UDP de mayúsculas escuchando en puerto %d\n", puerto);

    char buffer[2000];
    struct sockaddr_in client;
    socklen_t clientlen; /* no inicializamos aquí, lo haremos antes de cada recvfrom */

    /* Bucle principal: recibir datagramas y responder */
    while (1) {
        clientlen = sizeof(client); /* restaurar antes de cada llamada que lo modifica */
        ssize_t n = recvfrom(sock, buffer, sizeof(buffer)-1, 0, (struct sockaddr *)&client, &clientlen);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        /* Mostrar información del cliente */
        char ipstr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client.sin_addr, ipstr, sizeof(ipstr)) == NULL) {
            /* si falla, evitar crash y usar marcador */
            strncpy(ipstr, "unknown", sizeof(ipstr));
            ipstr[sizeof(ipstr)-1] = '\0';
        }
        printf("Recibidos %zd bytes de %s:%d\n", n, ipstr, ntohs(client.sin_port));

        /* Convertir a mayúsculas (solo la parte recibida) */
        for (ssize_t i = 0; i < n; ++i) {
            buffer[i] = toupper((unsigned char)buffer[i]);
        }

        /* Devolver la línea convertida al remitente */
        ssize_t sent = sendto(sock, buffer, n, 0, (struct sockaddr *)&client, clientlen);
        if (sent < 0) {
            perror("sendto");
        } else if (sent != n) {
            fprintf(stderr, "Aviso: se enviaron %zd bytes de %zd esperados\n", sent, n);
        }
    }

    close(sock);
    return 0;
}