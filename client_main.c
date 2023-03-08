#include "client_functions.h"

int main(int argc, char const* argv[])
{
    srand(time(NULL));

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

    map = calloc(5, sizeof(char*));

    if(!map){
        wprintw(win, "Memory fail\n");
        wgetch(win);
        return -5;
    }

    for(int i = 0; i < 5; ++i){
        (*(map + i)) = calloc(5, sizeof(char));

        if(!(*(map + i))){
            wprintw(win, "Memory fail\n");
            wgetch(win);

            for(int j = 0; j < i; ++j){
                free(*(map + j));
            }
            free(map);
        }
    }

    int sock = 0, valread, client_fd;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        wprintw(win, "\nInvalid address/ Address not supported \n");
        return -1;
    }

    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
        wprintw(win, "\nConnection Failed \n");
        return -1;
    }

//----------------------RZECZY Z SERWERA KTORE TU TEZ SIE PRZYDADZA------------------------------------


    int res, pid;
    int carry = 0, bank = 0, deaths = 0, campsite_x = 0, campsite_y = 0, player_x = 0, player_y = 0;

    for(int i = 0; i < 5; ++i) {
        read(sock, (*(map + i)), 5);
    }

    read(sock, &pid, sizeof(int));
    read(sock, &player_x, sizeof(int));
    read(sock, &player_y, sizeof(int));

    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 55, "Server's PID: %d", pid);
    wattroff(win, COLOR_PAIR(1));

    display_player_win(win, player_y, player_x);
    display_legend(win);

//-------------------------------KONIEC RZECZY Z SERWERA---------------------------------------------------------

    struct player_send *arg_send = calloc(1, sizeof(struct player_send));

    if(!arg_send){
        wprintw(win, "ERROR\n");
        for(int i = 0; i < 5; ++i){
            free(*(map + i));
        }
        free(map);
        endwin();
        return -1;
    }

    arg_send->win = win;
    arg_send->new_socket = sock;
    arg_send->disconnect = 0;

    if(campsite_x) {
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 2, 55, "Capsite X/Y: %d/%d\n", campsite_x, campsite_y);
        wattroff(win, COLOR_PAIR(1));
    }else{
        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 2, 55, "Campsite X/Y: unknown\n");
        wattroff(win, COLOR_PAIR(1));
    }

    int counter = 0;

    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 3, 55, "Round number: %d", counter);
    mvwprintw(win, 5, 55, "Player:");
    mvwprintw(win, 6, 55, "Number: %d", 2);
    mvwprintw(win, 7, 55, "Type: Human");
    mvwprintw(win, 8, 55, "Curr X/Y: %2d/%2d", player_x, player_y);
    mvwprintw(win, 9, 55, "Deaths: %2d", deaths);

    mvwprintw(win, 10, 55, "Carry: %3d", carry);
    mvwprintw(win, 11, 55, "Bank: %3d", bank);
    wattroff(win, COLOR_PAIR(1));

    pthread_t send;


    while(!arg_send->disconnect){
        pthread_create(&send, NULL, tsend, (void*)arg_send);

        int result_read_map = 0;

        for(int i = 0; i < 5; ++i) {
            result_read_map = read(sock, (*(map + i)), 5);
        }

        read(sock, &counter, sizeof(int));
        read(sock, &player_x, sizeof(int));
        read(sock, &player_y, sizeof(int));
        read(sock, &carry, sizeof(int));
        read(sock, &bank, sizeof(int));
        read(sock, &deaths, sizeof(int));
        read(sock, &campsite_x, sizeof(int));
        read(sock, &campsite_y, sizeof(int));


        if(player_x == -1)
            break;

        if(result_read_map)
            display_player_win(win, player_y, player_x);

        pthread_cancel(send);

        if(campsite_x) {
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, 2, 55, "Capsite X/Y: %d/%d\n", campsite_x, campsite_y);
            wattroff(win, COLOR_PAIR(1));
        }else{
            wattron(win, COLOR_PAIR(1));
            mvwprintw(win, 2, 55, "Campsite X/Y: unknown\n");
            wattroff(win, COLOR_PAIR(1));
        }

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 3, 55, "Round number: %d", counter);
        mvwprintw(win, 5, 55, "Player:");
        mvwprintw(win, 6, 55, "Number: %d", 2);
        mvwprintw(win, 7, 55, "Type: Human");
        mvwprintw(win, 8, 55, "Curr X/Y: %2d/%2d", player_x, player_y);
        mvwprintw(win, 9, 55, "Deaths: %2d", deaths);

        mvwprintw(win, 10, 55, "Carry: %3d", carry);
        mvwprintw(win, 11, 55, "Bank: %3d", bank);
        wattroff(win, COLOR_PAIR(1));

    }

    for(int i = 0; i < 30; ++i){
        for(int j = 0; j < 90; ++j){
            mvwprintw(win, i, j, " ");
        }
    }

    pthread_cancel(send);
    pthread_detach(send);

    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 7, "THE END");
    mvwprintw(win, 2, 1, "Press any key to exit");
    wattroff(win, COLOR_PAIR(1));

    wgetch(win);

    for(int i = 0; i < 5; ++i){
        free(*(map + i));
    }
    free(map);
    free(arg_send);
    endwin();

    close(client_fd);

    endwin();
    return 0;
}