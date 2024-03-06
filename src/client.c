#include <client.h>

int sockfd;
char client_socket_path[CLIENT_SOCKET_PATH_SIZE];

void *listen_server(void *arg) {
    int sockfd = *((int *)arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, '\0', BUFFER_SIZE);

        ssize_t received = recv(sockfd, buffer, BUFFER_SIZE, 0);

        switch (received)
        {
            case -1:
                perror("recv");
                exit(EXIT_FAILURE);
            case 0:
                printf("Server closed the connection. \n");
                break;
            
            default :
                if (strncmp(buffer, "FOUND", (size_t)received) == 0) {
                    printf("Number found. Exiting.\n");
                    exit(EXIT_SUCCESS);
                }
                if (strncmp(buffer, "CLIENT_FOUND", (size_t)received) == 0) {
                    printf("A client found the number. Exiting.\n");
                    exit(EXIT_SUCCESS);
                } else {
                    printf("Server message : %s.\n", buffer);
                }
        }
    }
    return NULL;
}


int main() {

    struct sockaddr_un server_addr;
    char buffer[BUFFER_SIZE];
    pthread_t listen_thread;

    memset(buffer, '\0', BUFFER_SIZE);

    if (snprintf(client_socket_path, sizeof(client_socket_path), "/tmp/client_socket_%d", getpid()) < 0) {
        perror("snprintf");
        return EXIT_FAILURE;
    }

    setup_signal_handling();

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket error");
        return EXIT_FAILURE;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    printf("Enter messages (CTRL+D to quit):\n");

    if (connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        exit(EXIT_FAILURE);
    }
    if(pthread_create(&listen_thread, NULL, listen_server, (void *)&sockfd) == -1) {
        perror("Could not create listening thread");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            perror("send error");
            break;
        }
    }

    pthread_join(listen_thread, NULL);
    cleanup();

    return EXIT_SUCCESS;
}

void cleanup() {
    if (close(sockfd) == -1){
        perror("close");
        exit(EXIT_FAILURE);
    }
    if (unlink(client_socket_path) == -1){
        perror("unlink");
        exit(EXIT_FAILURE);
    }
    printf("\nClient shutdown.\n");
}

void handle_signal() {
    cleanup();
    exit(EXIT_SUCCESS);
}

void setup_signal_handling() {

    struct sigaction sa;
    
    sa.sa_handler = handle_signal; 
    if (sigemptyset(&sa.sa_mask) == -1){
        perror("sigemtpyset");
        exit(EXIT_FAILURE);
    }

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