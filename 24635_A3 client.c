#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    printf("Connected to server.\n");

    while (1) {
        printf("\n Enter command (UP|text, LOW|text, REV|text) or 'bye':\n");

        fgets(input, BUFFER_SIZE, stdin);

        input[strcspn(input, "\n")] = 0;

        send(sock, input, strlen(input), 0);

        if (strcmp(input, "bye") == 0)
            break;

        memset(buffer, 0, BUFFER_SIZE);

        read(sock, buffer, BUFFER_SIZE);

        printf("Server response: %s\n", buffer);
    }

    close(sock);

    return 0;
}
