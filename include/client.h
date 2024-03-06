#ifndef CLIENT_H
#define CLIENT_H

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <pthread.h>
#include <unistd.h>

#define _XOPEN_SOURCE 700
#define BUFFER_SIZE 1024
#define CLIENT_SOCKET_PATH_SIZE 108
#define SERVER_SOCKET_PATH "/tmp/server_socket"

void cleanup();
void handle_signal();
void setup_signal_handling();

#endif