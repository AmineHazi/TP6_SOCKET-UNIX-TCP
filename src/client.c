#include <client.h>

int sockfd;
char client_socket_path[108];

int main() {
    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];

    memset(buffer, '\0', BUFFER_SIZE);

    snprintf(client_socket_path, sizeof(client_socket_path), "/tmp/client_socket_%d", getpid());

    setup_signal_handling();

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    printf("Enter messages (CTRL+D to quit):\n");

    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("send error");
            break;
        }

        memset(buffer, '\0', BUFFER_SIZE);
        ssize_t received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
        if (received < 0) {
            perror("recv error");
            break;
        } else if (received == 0) {
            printf("Server closed the connection.\n");
            break;
        } else {

            if (strncmp(buffer, "FOUND", (size_t) received) == 0) {
                printf("Number found. Exiting.\n");
                cleanup();
                return EXIT_SUCCESS;
            } else {
                printf("Server message : %s\n", buffer);
            }
        }
        memset(buffer, '\0', BUFFER_SIZE);
    }

    cleanup();

    return EXIT_SUCCESS;
}

void cleanup() {
    close(sockfd);
    unlink(client_socket_path);
    printf("\nClient shutdown.\n");
}

void handle_signal() {
    cleanup();
    exit(EXIT_SUCCESS);
}

void setup_signal_handling() {
    struct sigaction sa;
    sa.sa_handler = handle_signal; 
    sigemptyset(&sa.sa_mask); 
    sa.sa_flags = 0; 

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("Error setting up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}