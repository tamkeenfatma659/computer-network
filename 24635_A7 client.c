#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 2048

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    char request[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <roll_number>\n", argv[0]);
        return 1;
    }

    char *roll = argv[2];

    if (strlen(roll) != 9)
    {
        printf("Roll number must contain 9 digits.\n");
        return 1;
    }

    int last3 = atoi(roll + strlen(roll) - 3);
    int port = 30000 + last3;

    printf("Connecting to UDP server on port %d\n", port);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0)
    {
        printf("Invalid server IP address.\n");
        close(sockfd);
        return 1;
    }

    while (1)
    {
        printf("\nEnter command (UP/LOW/REV, SUM/MAX/MIN/AVG, "
               "or text for statistics; EXIT to stop):\n");

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';

        if (strlen(message) == 0)
            continue;

        snprintf(request, sizeof(request), "%s|%s", roll, message);

        if (sendto(sockfd, request, strlen(request), 0,
                   (struct sockaddr *)&server_addr,
                   sizeof(server_addr)) < 0)
        {
            perror("Send failed");
            continue;
        }

        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout));

        memset(buffer, 0, sizeof(buffer));

        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&server_addr, &server_len);

        if (n < 0)
        {
            printf("No response from server.\n");
            continue;
        }

        buffer[n] = '\0';
        printf("Server Response: %s\n", buffer);

        if (strcmp(message, "EXIT") == 0)
            break;
    }

    close(sockfd);
    return 0;
}