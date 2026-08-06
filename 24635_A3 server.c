#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT 8081
#define BUFFER_SIZE 1024

// Convert to uppercase
void to_upper(char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        str[i] = toupper(str[i]);
}

// Convert to lowercase
void to_lower(char *str) {
    for (int i = 0; str[i] != '\0'; i++)
        str[i] = tolower(str[i]);
}

// Reverse string
void reverse(char *str) {
    int i, len = strlen(str);
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    listen(server_fd, 3);

    printf("Server listening on port %d...\n", PORT);

    new_socket = accept(server_fd, (struct sockaddr *)&address,
                        (socklen_t *)&addrlen);

    printf("Client connected.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0)
            break;

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "bye") == 0) {
            printf("Client disconnected.\n");
            break;
        }

        char command[10], text[BUFFER_SIZE];

        sscanf(buffer, "%[^|]|%[^\n]", command, text);

        while (text[0] == ' ')
            memmove(text, text + 1, strlen(text));

        if (strcmp(command, "UP") == 0)
            to_upper(text);
        else if (strcmp(command, "LOW") == 0)
            to_lower(text);
        else if (strcmp(command, "REV") == 0)
            reverse(text);
        else
            strcpy(text, "Invalid Command");

        send(new_socket, text, strlen(text), 0);
    }

    close(new_socket);
    close(server_fd);

    printf("Server terminated.\n");

    return 0;
}
