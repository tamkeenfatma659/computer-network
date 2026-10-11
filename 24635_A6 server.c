#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    signal(SIGCHLD, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(1);
    }

    printf("Server is running on port %d...\n", PORT);

    while (1)
    {
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            close(client_fd);
        }
        else if (pid == 0)
        {
            /* Child process handles the client */
            close(server_fd);

            char student_id[50];
            char buffer[BUFFER_SIZE];
            char response[BUFFER_SIZE];
            char line[BUFFER_SIZE];
            int found = 0;

            memset(student_id, 0, sizeof(student_id));

            int n = recv(client_fd, student_id,
                         sizeof(student_id) - 1, 0);

            if (n > 0)
            {
                student_id[n] = '\0';

                FILE *fp = fopen("data.txt", "r");

                if (fp == NULL)
                {
                    snprintf(response, sizeof(response),
                             "Error: Cannot open data.txt");
                }
                else
                {
                    while (fgets(line, sizeof(line), fp))
                    {
                        char id[50];

                        if (sscanf(line, "%49s", id) == 1 &&
                            strcmp(id, student_id) == 0)
                        {
                            snprintf(response, sizeof(response),
                                     "Record Found: %. 990s", line);
                            found = 1;
                            break;
                        }
                    }

                    fclose(fp);

                    if (!found)
                    {
                        snprintf(response, sizeof(response),
                                 "Record Not Found");
                    }
                }

                send(client_fd, response, strlen(response), 0);
            }

            close(client_fd);
            exit(0);
        }
        else
        {
            /* Parent process continues accepting clients */
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}