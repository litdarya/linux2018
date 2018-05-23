#include <sys/socket.h>
#include <netinet/in.h>
#include <zconf.h>
#include <stdio.h>
#include <malloc.h>

int main() {
    int size = 10000;
    char* buff = (char*)malloc(size);
    int sock;
    int rc;
    struct sockaddr_in addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("can not create socket\n");
        return 0;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    rc = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if (rc != 0) {
        printf("can not connect\n");
        close(sock);
        return 0;
    }
    while(fgets (buff, size, stdin) != NULL) {
        rc = send(sock, buff, size, 0);
        if (rc == -1) {
            printf("can not send\n");
            break;
        }
        int len = recv(sock, buff, size, 0);
        printf("%.*s\n", len, buff);
    }
    close(sock);

    free(buff);
    return 0;
}