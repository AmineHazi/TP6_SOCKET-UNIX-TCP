#ifndef SERVER_H
#define SERVER_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define _XOPEN_SOURCE 700
#define LOG_DIR "logs"
#define LOG_FILE_PATH LOG_DIR "/server_log_%Y-%m-%d_%H-%M-%S.log"
#define SERVER_SOCKET_PATH "/tmp/server_socket"
#define BUFFER_SIZE 1024
#define MIN_NUM_RAND 1
#define MAX_NUM_RAND 1000
#define BACKLOG 10

void create_log_file_path();
void log_message_with_client_info(const char* prefix, int client_sock, int message);
void log_message(const char* message);
void cleanup();
void handle_signal();
void setup_signal_handling();
void new_thread(int service);
int random_number(int min_num, int max_num);
void* handle_client(void* arg);

#endif