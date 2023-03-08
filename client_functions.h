#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8989

char** map;

#ifndef GRA_FUNCTIONS_H
#define GRA_FUNCTIONS_H

struct player_send{
    WINDOW *win;
    int new_socket;
    int disconnect;
};


void *tsend(void *arg);
void display_legend(WINDOW* win);
void display_player_win(WINDOW* win, int player_y, int player_x);

#endif