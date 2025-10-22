#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Receptor UDP para array de floats
   Uso: receptor <puerto>
   Espera un único datagrama con formato:
   [uint32_t count (network order)] [float0] [float1] ...
   Converte os floats para host order e imprime o número de valores e os valores.
   Comentários em português.
*/

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    /* buffer grande suficiente para recibir datagrama completo */
    unsigned char buf[65536];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    ssize_t n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&sender, &sender_len);
    if (n < 0) {
        perror("recvfrom");
        close(sock);
        return 1;
    }

    if (n < 4) {
        fprintf(stderr, "Datagrama demasiado pequeno para conter count\n");
        close(sock);
        return 1;
    }

    /* obter count (network order -> host) */
    uint32_t cnt_net;
    memcpy(&cnt_net, buf, 4);
    uint32_t count = ntohl(cnt_net);

    /* calcular quantos floats realmente chegaram */
    ssize_t bytes_for_floats = n - 4;
    uint32_t floats_received = (uint32_t)(bytes_for_floats / 4);
    if (floats_received < count) {
        fprintf(stderr, "Aviso: se esperaba %u floats, pero llegaron %u (datagrama truncado)\n", count, floats_received);
    }

    /* imprimir IP y puerto del remitente */
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender.sin_addr, ipstr, sizeof(ipstr));
    printf("Recibidos %zd bytes de %s:%d\n", n, ipstr, ntohs(sender.sin_port));
    printf("Count declarado: %u\n", count);
    printf("Floats disponibles en el datagrama: %u\n", floats_received);

    /* convertir e imprimir los floats disponibles */
    for (uint32_t i = 0; i < floats_received; ++i) {
        uint32_t u;
        memcpy(&u, buf + 4 + i * 4, 4);
        u = ntohl(u);
        float f;
        memcpy(&f, &u, sizeof(f));
        printf("f[%u] = %f\n", i, f);
    }

    close(sock);
    return 0;
}