#include "serwer_functions.h"

#define PORT 8989

int main() {


    srand(time(NULL));

    if(get_maps()){
        printf("Error");
        return -2;
    }

    initscr();
    WINDOW *win = newwin(30, 90, 0, 0);

    start_color();

    init_pair(1, 15, 0);            //default
    init_pair(2, 9, 0);             //bestia
    init_pair(3, 22, 0);            //krzaki
    init_pair(4, 11, 0);            //coinsy
    init_pair(5, 26, 0);            //gracz
    init_pair(6, 3, 0);             //campsite
    init_pair(7, 8, 8);             //wall



    wrefresh(win);
    keypad(win, true);
    noecho();
    curs_set(0);
    cbreak();

//--------------KONIEC WCZYTYWANIA MAPY---------------------------------------------------------------------------------




    struct player **gracze = create_players(1);

    if(!gracze){
        printf("Brak pamieci");
        endwin();
        free_maps(NULL, NULL, NULL, NULL);
        return -2;
    }

    int number_of_players = 1;
    int *ptr_number_of_players = calloc(1, sizeof(int));
    *ptr_number_of_players = number_of_players;

    struct beast* bestia = add_beast();

    if(!bestia){
        printf("Brak pamieci");
        endwin();
        free_maps(gracze, NULL, NULL, NULL);
        return -2;
    }

    display_legend(win);


    struct arg_struct_beast *arg_beast = calloc(1, sizeof(struct arg_struct_beast));

    if(!arg_beast){
        printf("No memory\n");
        free_maps(gracze, bestia, NULL, NULL);
        endwin();
        return -1;
    }

    struct arg_struct_player *arg_player = calloc(1, sizeof(struct arg_struct_player));

    if(!arg_player){
        printf("No memory\n");
        free_maps(gracze, bestia, NULL, arg_beast);
        endwin();
        return -1;
    }

    struct arg_struct_accept *arg_accept = calloc(1, sizeof(struct arg_struct_accept));

    if(!arg_accept){
        printf("No memory\n");
        free_maps(gracze, bestia, arg_player, arg_beast);
        endwin();
        return -1;
    }

    struct arg_struct_client *arg_client = calloc(1, sizeof(struct arg_struct_client));

    if(!arg_accept){
        printf("No memory\n");
        free_maps(gracze, bestia, arg_player, arg_beast);
        free(arg_accept);
        endwin();
        return -1;
    }

    arg_beast->win = win;
    arg_beast->players = gracze;
    arg_beast->beast = bestia;

    arg_player->win = win;
    arg_player->player = *gracze;
    arg_player->players = gracze;


    pthread_t beast, player, accept_player, client, client_move;

//--------------KONIEC WSZYSTKICH ALOKACJI------------------------------------------------------------------------------
//--------------TWORZENIE SERWERA---------------------------------------------------------------------------------------



    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        free_maps(gracze, bestia, arg_player, arg_beast);
        free(arg_accept);
        free(arg_client);
        endwin();
        return -3;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        free_maps(gracze, bestia, arg_player, arg_beast);
        free(arg_accept);
        free(arg_client);
        endwin();
        return -3;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        free_maps(gracze, bestia, arg_player, arg_beast);
        free(arg_accept);
        free(arg_client);
        endwin();
        return -3;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        free_maps(gracze, bestia, arg_player, arg_beast);
        free(arg_accept);
        free(arg_client);
        endwin();
        return -3;
    }

    arg_accept->server_fd = server_fd;
    arg_accept->address = &address;
    arg_accept->addrlen = (size_t)sizeof(address);
    arg_accept->new_socket = -1;
    arg_accept->number_of_players = ptr_number_of_players;
    arg_accept->gracze = gracze;
    arg_accept->arg_client = arg_client;

    arg_client->win = win;
    arg_client->number_of_players = ptr_number_of_players;
    arg_client->players = gracze;
    arg_client->camp_x = 0;
    arg_client->camp_y = 0;

    int pid = getpid();
    arg_client->counter = 0;
    arg_accept->counter = 0;

    if((*(gracze))->camp_x) {
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 2, 55, "Capsite X/Y: %d/%d\n", (*(gracze))->camp_x, (*(gracze))->camp_y);
        wattroff(win, COLOR_PAIR(1));
    }else{
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 2, 55, "Campsite X/Y: unknown\n");
        wattroff(win, COLOR_PAIR(1));
    }

    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 55, "Server's PID: %d", pid);

    mvwprintw(win, 3, 55, "Round number: %d", arg_client->counter);
    mvwprintw(win, 5, 55, "Player:");
    mvwprintw(win, 6, 55, "Number: %d", 1);
    mvwprintw(win, 7, 55, "Type: Human");
    mvwprintw(win, 8, 55, "Curr X/Y: %2d/%2d", (*(gracze))->x, (*(gracze))->y);
    mvwprintw(win, 9, 55, "Deaths: %2d", (*(gracze))->deaths);

    mvwprintw(win, 10, 55, "Carry: %3d", (*(gracze))->carry);
    mvwprintw(win, 11, 55, "Bank: %3d", (*(gracze))->bank);
    wattroff(win, COLOR_PAIR(1));

    pthread_create(&accept_player, NULL, accept_player_socket, (void*)arg_accept);
    int thread_sent = 1;

    wrefresh(win);

    display_player_win(win, *(gracze));

    while(1){


        if(*ptr_number_of_players > 1)
            thread_sent = 0;

        if(*ptr_number_of_players == 1){
            if(thread_sent){
                pthread_cancel(accept_player);
            }
            pthread_create(&accept_player, NULL, accept_player_socket, (void*)arg_accept);
            thread_sent = 1;
        }

        pthread_create(&beast, NULL, beast_thread, arg_beast);
        pthread_join(beast, NULL);
        display_player_win(win, *gracze);

        if(*ptr_number_of_players > 1) {
            pthread_create(&client, NULL, client_send_map, (void *) arg_client);
            pthread_create(&client_move, NULL, client_thread, (void *) arg_client);
        }

        pthread_create(&player, NULL, player_thread, arg_player);


        if(*ptr_number_of_players > 1) {
            pthread_join(client, NULL);
        }

        usleep(250000);

        pthread_cancel(player);

        if(*ptr_number_of_players > 1){
            pthread_cancel(client_move);
        }

        if((*(gracze))->camp_x) {
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, 2, 55, "Capsite X/Y: %d/%d\n", (*(gracze))->camp_x, (*(gracze))->camp_y);
            wattroff(win, COLOR_PAIR(1));
        }else{
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, 2, 55, "Campsite X/Y: unknown\n");
            wattroff(win, COLOR_PAIR(1));
        }

        arg_client->counter++;
        arg_accept->counter++;

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 3, 55, "Round number: %d", arg_client->counter);
        mvwprintw(win, 5, 55, "Player:");
        mvwprintw(win, 6, 55, "Number: %d", 1);
        mvwprintw(win, 7, 55, "Type: Human");
        mvwprintw(win, 8, 55, "Curr X/Y: %2d/%2d", (*(gracze))->x, (*(gracze))->y);
        mvwprintw(win, 9, 55, "Deaths: %2d", (*(gracze))->deaths);

        mvwprintw(win, 10, 55, "Carry: %3d", (*(gracze))->carry);
        mvwprintw(win, 11, 55, "Bank: %3d", (*(gracze))->bank);
        wattroff(win, COLOR_PAIR(1));


        if((*gracze)->end_game == (int)'q') {
            break;
        }
    }

    if(*ptr_number_of_players > 1){
        (*(gracze + 1))->x = -1;

        pthread_create(&client, NULL, client_send_map, (void *) arg_client);
        pthread_join(client, NULL);
    }

    if(*ptr_number_of_players == 1 && thread_sent)
        pthread_cancel(accept_player);


    pthread_cancel(player);

    pthread_detach(beast);
    pthread_detach(player);
    pthread_detach(client);
    pthread_detach(client_move);
    pthread_detach(accept_player);

    close(arg_accept->new_socket);
    shutdown(server_fd, SHUT_RDWR);


    free(arg_accept);
    free(ptr_number_of_players);
    free(arg_client);
    free_maps(gracze, bestia, arg_player, arg_beast);
    endwin();

    return 0;
}