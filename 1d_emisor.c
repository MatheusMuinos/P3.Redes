#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Programa emissor UDP (envía array de floats)
   Uso: emisor <puerto_local> <ip_destino> <puerto_destino> <num_floats>
   O emissor gera num_floats valores float (1.0, 2.0, ...) e envia
   num_floats e os dados num único datagrama:
   [uint32_t count (network order)] [float0 (network order)] [float1] ...
   Comentários em português.
*/

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Uso: %s <puerto_local> <ip_destino> <puerto_destino> <num_floats>\n", argv[0]);
        return 1;
    }

    int local_port = atoi(argv[1]);
    const char *dest_ip = argv[2];
    int dest_port = atoi(argv[3]);
    int count = atoi(argv[4]);
    if (count <= 0) {
        fprintf(stderr, "num_floats deve ser > 0\n");
        return 1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* bind para usar porta local desejada */
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(local_port);
    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    /* preparar endereço destino */
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip, &dest.sin_addr) != 1) {
        fprintf(stderr, "IP destino inválida: %s\n", dest_ip);
        close(sock);
        return 1;
    }

    /* preparar buffer: 4 bytes para count + 4*count bytes para floats */
    size_t payload_size = 4 + (size_t)count * sizeof(uint32_t);
    if (payload_size > 65507) { /* UDP payload safe max */
        fprintf(stderr, "Demasiados floats: el datagrama sería mayor que el máximo UDP seguro\n");
        close(sock);
        return 1;
    }

    unsigned char *buf = malloc(payload_size);
    if (!buf) {
        perror("malloc");
        close(sock);
        return 1;
    }

    /* escrever count em network byte order (uint32_t) */
    uint32_t cnt_net = htonl((uint32_t)count);
    memcpy(buf, &cnt_net, 4);

    /* preencher floats: gerar valores de exemplo e convertir a network order */
    for (int i = 0; i < count; ++i) {
        float v = (float)(i + 1); /* exemplo: 1.0, 2.0, ... */
        uint32_t u;
        memcpy(&u, &v, sizeof(u));    /* interpretar float como uint32_t */
        u = htonl(u);                 /* converter endianess para network order */
        memcpy(buf + 4 + i * 4, &u, 4);
    }

    ssize_t sent = sendto(sock, buf, payload_size, 0, (struct sockaddr *)&dest, sizeof(dest));
    if (sent < 0) {
        perror("sendto");
        free(buf);
        close(sock);
        return 1;
    }

    printf("%zd bytes enviados (count=%d)\n", sent, count);

    free(buf);
    close(sock);
    return 0;
}