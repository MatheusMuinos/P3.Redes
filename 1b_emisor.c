#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Programa emissor UDP
   Uso: emisor <puerto_local> <ip_destino> <puerto_destino>
   Lê uma linha de stdin e envia para o destino especificado.
*/

int main(int argc, char *argv[]) {
    /* validação de argumentos */
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <puerto_local> <ip_destino> <puerto_destino>\n", argv[0]);
        return 1;
    }

    /* porta local para bind, IP destino e porta destino */
    int local_port = atoi(argv[1]);   /* porto local para bind */
    const char *dest_ip = argv[2];    /* IP de destino */
    int dest_port = atoi(argv[3]);    /* porto de destino */

    /* cria socket UDP (AF_INET + SOCK_DGRAM) */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    /* configura endereço local (todas as interfaces) e faz bind
       para garantir que o pacote saia com o porto local solicitado */
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY); /* todas as interfaces */
    local.sin_port = htons(local_port);        /* porta local a usar */

    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock);
        return 1;
    }

    /* prepara endereço do destino (estrutura sockaddr_in) */
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(dest_port);
    /* converte string IP para binário; inet_pton retorna 1 em sucesso */
    if (inet_pton(AF_INET, dest_ip, &dest.sin_addr) != 1) {
        fprintf(stderr, "IP destino inválida: %s\n", dest_ip);
        close(sock);
        return 1;
    }

    /* lê mensagem de stdin (até tamanho máximo seguro de payload UDP) */
    char buf[65507]; /* UDP payload safe max */
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        fprintf(stderr, "No se recibió mensaje por stdin\n");
        close(sock);
        return 1;
    }
    size_t len = strlen(buf);
    /* fgets deixa o newline; mantemos-o para enviar exatamente o que foi digitado */

    /* envia os dados para o destino usando sendto */
    ssize_t sent = sendto(sock, buf, len, 0, (struct sockaddr *)&dest, sizeof(dest));
    if (sent < 0) {
        perror("sendto");
        close(sock);
        return 1;
    }

    /* imprime número de bytes enviados e fecha o socket */
    printf("%zd bytes enviados\n", sent);

    close(sock);
    return 0;
}