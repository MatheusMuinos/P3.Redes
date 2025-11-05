#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    /* validação de argumentos: porta a escutar */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    /* cria socket UDP */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* configura endereço local para bind em todas as interfaces */
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY); /* aceita pacotes de qualquer interface */
    local.sin_port = htons(port);               /* porta a escutar */

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    /* buffer para receber dados e estruturas para obter info do remetente */
    char buf[65536];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    /* recvfrom bloqueia até chegar um pacote; retorna número de bytes recebidos */
    ssize_t n = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                         (struct sockaddr *)&sender, &sender_len);
    if (n < 0) {
        perror("recvfrom");
        close(sock);
        return 1;
    }
    buf[n] = '\0'; /* garante terminação de string para imprimir */

    /* converte endereço IPv4 binário para string legível */
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender.sin_addr, ipstr, sizeof(ipstr));

    /* imprime número de bytes, IP e porta do remetente e o conteúdo da mensagem */
    printf("Recibido %zd bytes de %s:%d\n", n, ipstr, ntohs(sender.sin_port));
    printf("Mensaje: %s\n", buf);

    close(sock);
    return 0;
}