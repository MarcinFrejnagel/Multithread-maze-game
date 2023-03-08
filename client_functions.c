#include "client_functions.h"

void display_legend(WINDOW* win){
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 15, 55, "Legend:\n");
    mvwprintw(win, 16, 59, " - players\n");
    mvwprintw(win, 17, 56, "    - wall\n");
    mvwprintw(win, 18, 56, "    - bushes (slow down)\n");
    mvwprintw(win, 19, 56, "    - wild beast\n");
    mvwprintw(win, 20, 56, "    - one coin\n");
    mvwprintw(win, 21, 56, "    - treasure (10 coins)\n");
    mvwprintw(win, 22, 56, "    - large treasure (50 coins)\n");
    mvwprintw(win, 23, 56, "    - campsite\n");
    mvwprintw(win, 24, 56, "    - dropped treasure\n");
    wattroff(win, COLOR_PAIR(1));

    wattron(win, COLOR_PAIR(5));
    mvwaddch(win, 16, 55, '1');
    mvwaddch(win, 16, 56, '2');
    mvwaddch(win, 16, 57, '3');
    mvwaddch(win, 16, 58, '4');
    wattroff(win, COLOR_PAIR(5));

    wattron(win, COLOR_PAIR(7));
    mvwaddch(win, 17, 55, 'X');
    wattroff(win, COLOR_PAIR(7));

    wattron(win, COLOR_PAIR(3));
    mvwaddch(win, 18, 55, '#');
    wattroff(win, COLOR_PAIR(3));

    wattron(win, COLOR_PAIR(2));
    wattron(win, A_BOLD);
    mvwaddch(win, 19, 55, '*');
    wattroff(win, A_BOLD);
    wattroff(win, COLOR_PAIR(2));
    wattroff(win, A_BOLD);

    wattron(win, COLOR_PAIR(4));
    mvwaddch(win, 20, 55, 'c');
    mvwaddch(win, 21, 55, 't');
    mvwaddch(win, 22, 55, 'T');
    mvwaddch(win, 23, 55, 'D');
    wattroff(win, COLOR_PAIR(4));

    wattron(win, COLOR_PAIR(6));
    mvwaddch(win, 24, 55, 'A');
    wattroff(win, COLOR_PAIR(6));
}

void display_player_win(WINDOW* win, int player_y, int player_x){

    for(int i = 0; i < 25; ++i){
        for(int j = 0; j < 52; ++j){
            wattron(win, COLOR_PAIR(1));
            mvwaddch(win, i, j, ' ');
            wattroff(win, COLOR_PAIR(1));
        }
    }

    int x = player_x - 2;
    int y = player_y - 2;

    if(x <= 0)
        x = 0;

    if(y <= 0)
        y = 0;

    if(y > 20)
        y = 20;

    if(x > 46)
        x = 46;

    for(int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {

            if (*(*(map + i) + j) == '#') {
                wattron(win, COLOR_PAIR(3));
                mvwaddch(win, i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(3));
            } else if (*(*(map + i) + j) == 't' || *(*(map + i) + j) == 'T' || *(*(map + i) + j) == 'c' ||
                       *(*(map + i) + j) == 'D') {
                wattron(win, COLOR_PAIR(4));
                mvwaddch(win,  i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(4));
            } else if (*(*(map + i) + j) == 'A') {
                wattron(win, COLOR_PAIR(6));
                mvwaddch(win, i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(6));
            } else if (*(*(map + i) + j) == 'X') {
                wattron(win, COLOR_PAIR(7));
                mvwaddch(win,  i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(7));
            } else if (*(*(map + i) + j) == '1' || *(*(map + i) + j) == '2'){
                wattron(win, COLOR_PAIR(5));
                mvwaddch(win, i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(5));
            }else if (*(*(map + i) + j) == '*'){
                wattron(win, COLOR_PAIR(2));
                wattron(win, A_BOLD);
                mvwaddch(win, i + y, j + x, *(*(map + i) + j));
                wattroff(win, A_BOLD);
                wattroff(win, COLOR_PAIR(2));
            }else{
                wattron(win, COLOR_PAIR(1));
                mvwaddch(win, i + y, j + x, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(1));
            }
        }
    }

}

void *tsend(void *arg){
    if(arg == NULL){
        return NULL;
    }

    struct player_send *new = (struct player_send*)(arg);

    flushinp();
    int c = wgetch(new->win);

    send(new->new_socket, &c, sizeof(c), 0);

    if(tolower(c) == (int)'q'){
        new->disconnect = 1;
    }

    return NULL;
}
