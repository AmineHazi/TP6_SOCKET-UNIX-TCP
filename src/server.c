#include <server.h>

char log_file_path[256]; 
int sockfd;
ClientNode *head = NULL; 
pthread_mutex_t list_mutex; 
int random;

int main() {

    switch (fork()) {
        case 0:

            if(setsid() < 0){
                perror("setsid");
                exit(EXIT_FAILURE);
            }

            struct sockaddr_un server_addr, client_addr;
            char buffer[BUFFER_SIZE];
            memset(buffer, '\0', BUFFER_SIZE); 
            socklen_t addr_size;
            
            mkdir(LOG_DIR, 0755);
            setup_signal_handling();
            random = random_number(MIN_NUM_RAND, MAX_NUM_RAND);
            create_log_file_path();
            if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
                perror("socket error");
                exit(EXIT_FAILURE);
            }
            log_message("Socket created.");

            memset(&server_addr, 0, sizeof(server_addr));
            server_addr.sun_family = AF_UNIX;
            strncpy(server_addr.sun_path, SERVER_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);
            unlink(SERVER_SOCKET_PATH); 

            if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                perror("bind error");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            log_message("Socket bound to address.");

            log_message("Server is waiting for messages...");

            if (listen(sockfd, BACKLOG) == -1){
                perror("listen");
                exit(EXIT_FAILURE);
            }

            while (1) {
                addr_size = sizeof(client_addr);
                int service = accept(sockfd, (struct sockaddr*) &client_addr, &addr_size);
                if (service < 0) {
                    perror("accept error");
                    log_message("Error receiving message.");
                    continue;
                }

                new_thread(service);
                memset(buffer, '\0', BUFFER_SIZE); 
            }

        default:
            break;
    }

    return EXIT_SUCCESS;
}

/*Fonction pour les logs*/
void create_log_file_path() {
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    strftime(log_file_path, sizeof(log_file_path), LOG_FILE_PATH, tm_now);
}

void log_message_with_client_info(const char* prefix, int client_sock, int message) {
    FILE* log_file = fopen(log_file_path, "a");
    if (log_file != NULL) {
        time_t now = time(NULL);
        char formatted_time[64];
        strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M:%S", localtime(&now));

        fprintf(log_file, "[%s] (%d) %s: %d\n", formatted_time, client_sock , prefix, message);
        fclose(log_file);
    } else {
        perror("Failed to open log file");
    }
}

void log_message(const char* message) {
    FILE* log_file = fopen(log_file_path, "a");
    if (log_file != NULL) {
        fprintf(log_file, "%s\n", message);
        fclose(log_file); 
    } else {
        perror("Failed to open log file");
    }
}

/*Netoyer les resource*/
void cleanup() {
    close(sockfd);
    unlink(SERVER_SOCKET_PATH);
    log_message("Server shutdown.");
}

/*Gestion des signaux*/
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
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigation");
        exit(EXIT_FAILURE);
    }
}

/*Creation du thread*/
void new_thread(int socket) {
    int ret;

    pthread_attr_t attr;
    if ((ret = pthread_attr_init(&attr)) != 0) {
        perror("pthread_attr_init");
        exit(EXIT_FAILURE);
    }

    if ((ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
        perror("pthread_attr_setdetachstate");
        exit(EXIT_FAILURE);
    }

    int* socketp = malloc(sizeof (int));
    *socketp = socket;
    pthread_t t;
    if ((ret = pthread_create(&t, &attr, handle_client, (void*) socketp)) != 0) {
        perror("pthead_create");
        exit(EXIT_FAILURE);
    }

    if ((ret = pthread_attr_destroy(&attr)) != 0) {
        perror("pthread_attr_destroy");
        exit(EXIT_FAILURE);
    }
}


void* handle_client(void* arg) {

    int socket = *((int *)arg);
    add_client(socket);
    ssize_t n;
    char buf[BUFFER_SIZE];

    log_message_with_client_info("Random number for client", socket , random);

    while((n = recv(socket, buf, BUFFER_SIZE - 1, 0)) > 0) {

        int num = (int) strtol(buf, NULL, 10);

        log_message_with_client_info("Message recieved", socket, num);

        if(num == random){
            log_message_with_client_info("Number found", socket, num);
            const char* msg = "FOUND";
            if(send(socket, msg, strlen(msg), 0) < 0){
                perror("send");
            }
            remove_client(socket);
            notify_all_clients("CLIENT_FOUND");
        } else {
            if(num < random) {
                const char* msg = "C'est plus !";
                if(send(socket, msg, strlen(msg), 0) < 0){
                    perror("send");
                }
            } else {
                const char* msg = "C'est moin !";
                if(send(socket, msg, strlen(msg), 0) < 0){
                perror("send");
                }   
            }
        }
    }

    if(close(socket) == -1){
        perror("close");
    }

    free(arg);
    pthread_exit(NULL);
}

int random_number(int min_num, int max_num) {
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num) {
        low_num = min_num;
        hi_num = max_num + 1; 
    } else {
        low_num = max_num + 1; 
        hi_num = min_num;
    }

    srand((unsigned int)time(NULL));
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}

void add_client(int socket) {
    pthread_mutex_lock(&list_mutex);
    ClientNode *newNode = malloc(sizeof(ClientNode));
    newNode->socket = socket;
    newNode->next = head;
    head = newNode;
    pthread_mutex_unlock(&list_mutex);
}

void remove_client(int socket) {
    pthread_mutex_lock(&list_mutex);
    ClientNode *current = head;
    ClientNode *prev = NULL;

    while (current != NULL) {
        if (current->socket == socket) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }

            free(current);
            break; 
        }
        prev = current;
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
}

void notify_all_clients(const char *msg) {
    pthread_mutex_lock(&list_mutex);
    ClientNode *current = head;

    while (current != NULL) {

        if(send(current->socket, msg, strlen(msg), 0) == -1){
            perror("send");
        }
        current = current->next;
    }

    pthread_mutex_unlock(&list_mutex);
}
