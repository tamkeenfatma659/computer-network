#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define ROLL "241050003"
#define PORT 30003
#define BUFFER_SIZE 2048

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char message[BUFFER_SIZE];

    int last3 = atoi(ROLL + strlen(ROLL) - 3);
    int d = (last3 / 100 + (last3 / 10) % 10 + last3 % 10) % 3;

    printf("Service Type: Type %c\n", 'A' + d);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(1);
    }

    printf("UDP Server running on port %d\n", PORT);

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        memset(response, 0, sizeof(response));

        int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                         (struct sockaddr *)&client_addr, &client_len);

        if (n < 0)
        {
            perror("Receive failed");
            continue;
        }

        buffer[n] = '\0';

        char *sep = strchr(buffer, '|');

        if (sep == NULL)
        {
            strcpy(response, "ERROR: Invalid message format");
        }
        else
        {
            *sep = '\0';

            printf("\nClient: %s\n", inet_ntoa(client_addr.sin_addr));
            printf("Received: %s\n", sep + 1);

            if (strcmp(buffer, ROLL) != 0)
            {
                strcpy(response, "ERROR: Unauthorized client");
            }
            else
            {
                strcpy(message, sep + 1);

                if (strcmp(message, "EXIT") == 0)
                {
                    strcpy(response, "Server shutting down");
                    sendto(sockfd, response, strlen(response), 0,
                           (struct sockaddr *)&client_addr, client_len);

                    printf("Exit command received. Shutting down.\n");
                    close(sockfd);
                    return 0;
                }

                if (d == 0)
                {
                    char *p = strchr(message, ' ');

                    if (p == NULL)
                    {
                        strcpy(response, "ERROR: Use OP|TEXT");
                    }
                    else
                    {
                        *p = '\0';
                        char *text = p + 1;

                        if (strcmp(message, "UP") == 0)
                        {
                            for (int i = 0; text[i]; i++)
                                text[i] = toupper((unsigned char)text[i]);
                        }
                        else if (strcmp(message, "LOW") == 0)
                        {
                            for (int i = 0; text[i]; i++)
                                text[i] = tolower((unsigned char)text[i]);
                        }
                        else if (strcmp(message, "REV") == 0)
                        {
                            int len = strlen(text);

                            for (int i = 0; i < len / 2; i++)
                            {
                                char temp = text[i];
                                text[i] = text[len - 1 - i];
                                text[len - 1 - i] = temp;
                            }
                        }
                        else
                        {
                            strcpy(response, "ERROR: Invalid operation");
                        }

                        if (response[0] == '\0')
                            snprintf(response, sizeof(response), "%s", text);
                    }
                }
                else if (d == 1)
                {
                    char op[20];
                    char *p = strchr(message, ' ');

                    if (p == NULL)
                    {
                        strcpy(response, "ERROR: Use OP|n|numbers");
                    }
                    else
                    {
                        *p = '\0';
                        strcpy(op, message);

                        char *token = strtok(p + 1, " ");
                        int count = 0;
                        long long sum = 0;
                        int min = 0, max = 0;
                        int valid = 1;

                        while (token != NULL)
                        {
                            char *end;
                            long value = strtol(token, &end, 10);

                            if (*token == '\0' || *end != '\0')
                            {
                                valid = 0;
                                break;
                            }

                            if (count == 0)
                            {
                                min = max = (int)value;
                            }

                            if (value < min)
                                min = (int)value;

                            if (value > max)
                                max = (int)value;

                            sum += value;
                            count++;

                            token = strtok(NULL, " ");
                        }

                        if (!valid || count == 0)
                        {
                            strcpy(response, "ERROR: Invalid integer list");
                        }
                        else if (strcmp(op, "SUM") == 0)
                        {
                            snprintf(response, sizeof(response),
                                     "%lld", sum);
                        }
                        else if (strcmp(op, "MAX") == 0)
                        {
                            snprintf(response, sizeof(response),
                                     "%d", max);
                        }
                        else if (strcmp(op, "MIN") == 0)
                        {
                            snprintf(response, sizeof(response),
                                     "%d", min);
                        }
                        else if (strcmp(op, "AVG") == 0)
                        {
                            snprintf(response, sizeof(response),
                                     "%.2f", (double)sum / count);
                        }
                        else
                        {
                            strcpy(response, "ERROR: Invalid operation");
                        }
                    }
                }
                else
                {
                    int letters = 0, vowels = 0;
                    int consonants = 0, digits = 0, spaces = 0;

                    for (int i = 0; message[i]; i++)
                    {
                        unsigned char ch = (unsigned char)message[i];

                        if (isalpha(ch))
                        {
                            letters++;

                            char c = tolower(ch);

                            if (c == 'a' || c == 'e' ||
                                c == 'i' || c == 'o' || c == 'u')
                                vowels++;
                            else
                                consonants++;
                        }
                        else if (isdigit(ch))
                        {
                            digits++;
                        }
                        else if (ch == ' ')
                        {
                            spaces++;
                        }
                    }

                    snprintf(response, sizeof(response),
                             "LETTERS=%d;VOWELS=%d;CONSONANTS=%d;"
                             "DIGITS=%d;SPACES=%d",
                             letters, vowels, consonants, digits, spaces);
                }
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_len);

        printf("Response: %s\n", response);
    }

    close(sockfd);
    return 0;
}