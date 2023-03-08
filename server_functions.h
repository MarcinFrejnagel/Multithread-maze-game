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

int** drop_map;
char** map;

#ifndef GRA_FUNCTIONS_H
#define GRA_FUNCTIONS_H

struct player{
    int spawn_x;
    int spawn_y;
    int x;
    int y;
    int deaths;
    int bank;
    int carry;
    int is_on_something;
    char block;
    int slow;
    char symbol;
    int end_game;
    int camp_x;
    int camp_y;
};

enum directions{
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3,
    RANDOM
};

struct beast{
    int x;
    int y;
    int is_on_something;
    char block;
    int slow;
};

struct arg_struct_beast{
    struct beast *beast;
    struct player **players;
    WINDOW *win;
};

struct arg_struct_player{
    struct player *player;
    struct player **players;
    WINDOW *win;
};

struct arg_struct_client{
    struct player *player;
    int new_socket;
    WINDOW *win;
    int *number_of_players;
    struct player **players;
    int camp_x;
    int camp_y;
    int counter;
};

struct arg_struct_accept{
    int server_fd;
    struct sockaddr_in* address;
    size_t addrlen;
    int new_socket;
    struct player **gracze;
    struct arg_struct_client* arg_client;
    int *number_of_players;
    int counter;
    WINDOW *win;
};

void* accept_player_socket(void* arg);

enum directions find_player(struct beast* beast);
int beast_move_up(WINDOW *win, struct beast* beast, struct player** player);
int beast_move_down(WINDOW *win, struct beast* beast, struct player** player);
int beast_move_right(WINDOW *win, struct beast* beast, struct player** player);
int beast_move_left(WINDOW *win, struct beast* beast, struct player** player);
int beast_movement(WINDOW *win, struct beast* beast, struct player** player);
void* beast_thread(void* arg);

void spawn_treasure(WINDOW *win, char c);

void kill_player(WINDOW* win, struct player* player);
int move_up(WINDOW *win, struct player* player);
int move_down(WINDOW *win, struct player* player);
int move_right(WINDOW *win, struct player* player);
int move_left(WINDOW *win, struct player* player);
void collect_coin(struct player* player, char treasure);
void bank(struct player* player);

void* client_send_map(void* arg);
void* client_thread(void* arg);

void* player_thread(void *arg);
int player_movement(WINDOW *win, struct player* player);
struct player** create_players(int num);
struct player* add_player();
struct beast* add_beast();
int get_maps();
void free_maps(struct player **players, struct beast *beast, struct arg_struct_player* arg_player, struct arg_struct_beast *arg_beast);
void display_legend(WINDOW* win);
void display_player_win(WINDOW* win, struct player* player);

#endif
