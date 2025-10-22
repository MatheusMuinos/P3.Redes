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

    /* buffer pequeno para provocar truncamento (ex.: 1000 bytes) */
    char buf[1000];
    struct sockaddr_in sender;
    socklen_t sender_len = sizeof(sender);

    /* configurar recvmsg para poder inspecionar msg_flags (p.ex. MSG_TRUNC) */
    struct iovec iov;
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf) - 1; /* deixar espaço para terminador */

    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = &sender;
    msg.msg_namelen = sender_len;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    /* recvmsg bloqueia e devuelve número de bytes armazenados em buf */
    ssize_t n = recvmsg(sock, &msg, 0);
    if (n < 0) {
        perror("recvmsg");
        close(sock);
        return 1;
    }

    /* actualizar sender_len com o tamanho real obtido e terminar string */
    sender_len = msg.msg_namelen;
    buf[(n < (ssize_t)sizeof(buf)) ? n : (sizeof(buf)-1)] = '\0';

    /* obter IP e porta do remetente */
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &sender.sin_addr, ipstr, sizeof(ipstr));

    /* imprimir informação sobre truncamento e conteúdo recebido */
    printf("Recebidos %zd bytes armazenados no buffer\n", n);
    printf("De %s:%d\n", ipstr, ntohs(sender.sin_port));
    if (msg.msg_flags & MSG_TRUNC) {
        printf("ATENÇÃO: datagrama foi truncado (MSG_TRUNC set). Os bytes restantes foram descartados.\n");
    } else {
        printf("Datagrama recebido sem truncamento.\n");
    }
    printf("Conteúdo armazenado (até %zu bytes):\n%s\n", sizeof(buf)-1, buf);

    close(sock);
    return 0;
}