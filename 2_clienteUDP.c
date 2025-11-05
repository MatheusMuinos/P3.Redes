#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

/* Cliente UDP que lee un archivo línea a línea, envía cada línea al servidor UDP,
   recibe la línea convertida a mayúsculas y la escribe en un archivo de salida.
   Uso: clienteUDP <archivo_entrada> <ip_servidor> <puerto_servidor>
   Comentarios en portugués.
*/

static void build_output_name(const char *in, char *out, size_t outsz) {
    /* constrói nome de saída baseado em in, inserindo _MAYUS antes da extensão */
    const char *dot = strrchr(in, '.');
    if (!dot) {
        snprintf(out, outsz, "%s_MAYUS", in);
    } else {
        size_t base = (size_t)(dot - in);
        if (base >= outsz) base = outsz - 1;
        memcpy(out, in, base);
        out[base] = '\0';
        snprintf(out + base, outsz - base, "_MAYUS%s", dot);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> <ip_servidor> <puerto>\n", argv[0]);
        return 1;
    }

    const char *nome_entrada = argv[1];
    const char *ip_servidor = argv[2];
    int puerto = atoi(argv[3]);

    /* abrir arquivos */
    FILE *fin = fopen(nome_entrada, "r");
    if (!fin) { perror("Erro ao abrir arquivo de entrada"); return 1; }

    char nome_saida[512];
    build_output_name(nome_entrada, nome_saida, sizeof(nome_saida));
    FILE *fout = fopen(nome_saida, "w");
    if (!fout) { perror("Erro ao abrir arquivo de saída"); fclose(fin); return 1; }

    /* crear socket UDP y bind a puerto ephemeral para recibir respuestas */
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); fclose(fin); fclose(fout); return 1; }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(0); /* dejar que el SO asigne puerto */
    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("bind");
        close(sock); fclose(fin); fclose(fout);
        return 1;
    }

    /* preparar dirección del servidor */
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    inet_pton(AF_INET, ip_servidor, &server.sin_addr);
    server.sin_port = htons(puerto);

    /* configurar timeout de recepción */
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    char linha[1001];
    char resposta[2000];
    socklen_t server_len = sizeof(server);

    /* enviar línea por línea y recibir la respuesta para escribirla en el archivo de salida */
    while (fgets(linha, sizeof(linha), fin)) {
        size_t tam = strlen(linha);
        /* Enviar la línea al servidor */
        ssize_t sent = sendto(sock, linha, tam, 0, (struct sockaddr *)&server, sizeof(server));
        if (sent < 0) {
            perror("Erro ao enviar linha");
            break;
        }

        /* esperar la respuesta del servidor */
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t n = recvfrom(sock, resposta, sizeof(resposta)-1, 0, (struct sockaddr *)&from, &fromlen);
        if (n < 0) {
            perror("Erro ao receber resposta (timeout ou otro)");
            break;
        }
        resposta[n] = '\0';
        // escribir la linea
        fputs(resposta, fout);

        sleep(10);  // Esperar 10 segundo antes de enviar la siguiente línea (Ej.3)
    }

    close(sock);
    fclose(fin);
    fclose(fout);
    printf("Arquivo convertido salvo como: %s\n", nome_saida);
    return 0;
}