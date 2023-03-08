#include "server_functions.h"

#define yMax = 52;
#define xMax = 25;

pthread_mutex_t map_mut;

void* accept_player_socket(void* arg){
    if(!arg)
        return NULL;

    struct arg_struct_accept *temp = (struct arg_struct_accept*)arg;

    if ((temp->new_socket = accept(temp->server_fd, (struct sockaddr*)&temp->address, (socklen_t*)&temp->addrlen)) < 0) {
        return NULL;
    }

    (*(temp->gracze + *temp->number_of_players)) = add_player();
    (*(temp->gracze + *temp->number_of_players))->symbol = (char)('0' + *temp->number_of_players + 1);

    map[(*(temp->gracze + *temp->number_of_players))->y][(*(temp->gracze + *temp->number_of_players))->x] = (*(temp->gracze + *temp->number_of_players))->symbol;

    int x = (*(temp->gracze + 1))->x - 2;
    int y = (*(temp->gracze + 1))->y - 2;


    if (x <= 0)
        x = 0;
    else if (x > 46)
        x = 46;

    if (y <= 0)
        y = 0;
    else if (y > 20)
        y = 20;

    char player_map[5][5];

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            player_map[i][j] = *(*(map + i + y) + j + x);
        }
    }
    for (int i = 0; i < 5; ++i) {
        send(temp->new_socket, player_map[i], 5, 0);
    }

    int pid = getpid();

    send(temp->new_socket, &pid, sizeof(int), 0);
    send(temp->new_socket, &(*(temp->gracze + *(temp->number_of_players)))->x, sizeof(int), 0);
    send(temp->new_socket, &(*(temp->gracze + *(temp->number_of_players)))->y, sizeof(int), 0);

    temp->arg_client->player = (*(temp->gracze + *temp->number_of_players));
    temp->arg_client->new_socket = temp->new_socket;
    (*temp->number_of_players)++;

    return NULL;
}

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

struct player* add_player(){
    struct player* result = (struct player*) calloc(1, sizeof(struct player));

    if(!result)
        return NULL;

    while(1){
        result->spawn_x = rand()%52;
        result->spawn_y = rand()%25;

        if(map[result->spawn_y][result->spawn_x] == ' ') {
            break;
        }
    }

    result->x = result->spawn_x;
    result->y = result->spawn_y;
    result->carry = 0;
    result->bank = 0;
    result->deaths = 0;
    result->is_on_something = 0;
    result->block = ' ';
    result->slow = 0;
    result->end_game = 0;
    result->camp_x = 0;
    result->camp_y = 0;

    return result;
}

struct beast* add_beast(){
    struct beast* result = (struct beast*) calloc(1, sizeof(struct beast));

    if(!result)
        return NULL;

    while(1){
        result->x = rand()%52;
        result->y = rand()%25;

        if(map[result->y][result->x] == ' ') {
            map[result->y][result->x] = '*';
            break;
        }
    }

    result->is_on_something = 0;
    result->block = ' ';
    result->slow = 0;

    return result;
}

void bank(struct player* player){
    player->bank += player->carry;
    player->carry = 0;
}

void spawn_treasure(WINDOW *win, char c){
    while(1){
        int x = rand()%52;
        int y = rand()%25;

        if(map[y][x] == ' '){
            map[y][x] = c;
            break;
        }
    }
}

void collect_coin(struct player* player, char treasure){
    if(treasure == 'c')
        player->carry++;
    else if(treasure == 't')
        player->carry+=10;
    else if(treasure == 'T')
        player->carry+=50;
    else if(treasure == 'D') {
        if(drop_map[player->y][player->x] < 0){
            player->is_on_something = 1;

            if(player->y == 12 && player->x == 23) {
                player->block = 'A';
            }else {
                player->block = '#';
                player->slow = 1;
            }
        }
        player->carry += abs(drop_map[player->y][player->x]);
        drop_map[player->y][player->x] = 0;
    }
}

void display_player_win(WINDOW* win, struct player* player){
    /*int x = player->x - 2;
    int y = player->y - 2;

    for(int i = 0; i < 25; ++i){
        for(int j = 0; j < 51; ++j){
            wattron(win, COLOR_PAIR(1));
            mvwaddch(win, i, j, ' ');
            wattroff(win, COLOR_PAIR(1));
        }
    }

    if(x < 0)
        x = 0;

    if(y < 0)
        y = 0;

    if(y > 20)
        y = 20;

    if(x > 46)
        x = 46;

    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < 5; ++j){
            if(*(*(map + y + i) + x + j) == '#'){
                wattron(win, COLOR_PAIR(3));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(3));
            }else if(*(*(map + y + i) + x + j) == 't' || *(*(map + y + i) + x + j) == 'T' || *(*(map + y + i) + x + j) == 'c' || *(*(map + y + i) + x + j) == 'D'){
                wattron(win, COLOR_PAIR(4));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(4));
            }else if(*(*(map + y + i) + x + j) == 'A'){
                wattron(win, COLOR_PAIR(6));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                player->camp_x = x + j;
                player->camp_y = y + i;
                wattroff(win, COLOR_PAIR(6));
            }else if(*(*(map + y + i) + x + j) == 'X'){
                wattron(win, COLOR_PAIR(7));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(7));
            }else if(*(*(map + y + i) + x + j) == '*'){
                wattron(win, COLOR_PAIR(2));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(2));
            }else if(*(*(map + y + i) + x + j) == '1' || *(*(map + y + i) + x + j) == '2'){
                wattron(win, COLOR_PAIR(5));
                mvwaddch(win, i + y, x + j, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(5));
            }else{
                wattron(win, COLOR_PAIR(1));
                mvwaddch(win, i + y, j + x, *(*(map + y + i) + x + j));
                wattroff(win, COLOR_PAIR(1));
            }
        }
    }*/
    
    int x = player->x - 2;
    int y = player->y - 2;

    for(int i = 0; i < 25; ++i){
        for(int j = 0; j < 51; ++j){
            wattron(win, COLOR_PAIR(1));
            mvwaddch(win, i, j, ' ');
            wattroff(win, COLOR_PAIR(1));
        }
    }

    if(x < 0)
        x = 0;

    if(y < 0)
        y = 0;

    if(y > 20)
        y = 20;

    if(x > 46)
        x = 46;

    for(int i = 0; i < 5; ++i){
        for(int j = 0; j < 5; ++j){
            if(*(*(map + y + i) + x + j) == 'A'){
                player->camp_x = x + j;
                player->camp_y = y + i;
            }
        }
    }

    for(int i = 0; i < 25; ++i){
        for(int j = 0; j < 51; ++j){
            if(*(*(map + i) + j) == '#'){
                wattron(win, COLOR_PAIR(3));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(3));
            }else if(*(*(map + i) + j) == 't' || *(*(map + i) + j) == 'T' || *(*(map + i) + j) == 'c' || *(*(map + i) + j) == 'D'){
                wattron(win, COLOR_PAIR(4));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(4));
            }else if(*(*(map + i) + j) == 'A'){
                wattron(win, COLOR_PAIR(6));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(6));
            }else if(*(*(map + i) + j) == 'X'){
                wattron(win, COLOR_PAIR(7));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(7));
            }else if(*(*(map + i) + j) == '*'){
                wattron(win, COLOR_PAIR(2));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(2));
            }else if(*(*(map + i) + j) == '1' || *(*(map + i) + j) == '2'){
                wattron(win, COLOR_PAIR(5));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(5));
            }else{
                wattron(win, COLOR_PAIR(1));
                mvwaddch(win, i, j, *(*(map + i) + j));
                wattroff(win, COLOR_PAIR(1));
            }
        }
    }

}



enum directions find_player(struct beast* beast) {
    enum directions result = RANDOM;
    int distance = 0;
    int found_player = 0;
    int can_check_first_cross = 1;
    int can_check_second_cross = 1;

    for (int i = 1; map[beast->y][beast->x + i] != 'X' && !found_player; ++i) {


        if(map[beast->y-1][beast->x+i] == 'X')
            can_check_first_cross = 0;

        for (int j = 1; map[beast->y - j][beast->x + i + j - 1] != 'X' && can_check_first_cross; ++j) {

            if (map[beast->y - j][beast->x + i + j - 1] == '1' || map[beast->y - j][beast->x + i + j - 1] == '2' ||
                map[beast->y - j][beast->x + i + j - 1] == '3' || map[beast->y - j][beast->x + i + j - 1] == '4') {
                if (!distance || (i + j) < distance) {
                    result = RIGHT;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }

        if(map[beast->y+1][beast->x+i] == 'X')
            can_check_second_cross = 0;


        for (int j = 1; map[beast->y + j][beast->x + i + j - 1] != 'X' && can_check_second_cross; ++j) {

            if (map[beast->y + j][beast->x + i + j - 1] == '1' || map[beast->y + j][beast->x + i + j - 1] == '2' ||
                map[beast->y + j][beast->x + i + j - 1] == '3' || map[beast->y + j][beast->x + i + j - 1] == '4') {
                if (!distance || (i + j) < distance) {
                    result = RIGHT;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }


        if (map[beast->y][beast->x + i] == '1' || map[beast->y][beast->x + i] == '2' ||
            map[beast->y][beast->x + i] == '3' || map[beast->y][beast->x + i] == '4') {
            if (!distance || i < distance) {
                result = RIGHT;
                distance = i;
            }
            found_player = 1;
            break;
        }
    }

    found_player = 0, can_check_first_cross = 1, can_check_second_cross = 1;


    for (int i = 1; map[beast->y][beast->x - i] != 'X' && !found_player; ++i) {

        if(map[beast->y-1][beast->x-i] == 'X')
            can_check_first_cross = 0;

        for (int j = 1; map[beast->y - j][beast->x - i - j + 1] != 'X' && can_check_first_cross; ++j) {

            if (map[beast->y - j][beast->x - i - j + 1] == '1' || map[beast->y - j][beast->x - i - j + 1] == '2' ||
                map[beast->y - j][beast->x - i - j + 1] == '3' || map[beast->y - j][beast->x - i - j + 1] == '4') {
                if (!distance || (i + j) < distance) {
                    result = LEFT;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }

        if(map[beast->y+1][beast->x-i] == 'X')
            can_check_second_cross = 0;


        for (int j = 1; map[beast->y + j][beast->x - i - j + 1] != 'X' && can_check_second_cross; ++j) {

            if (map[beast->y + j][beast->x - i - j + 1] == '1' || map[beast->y + j][beast->x - i - j + 1] == '2' ||
                map[beast->y + j][beast->x - i - j + 1] == '3' || map[beast->y + j][beast->x - i - j + 1] == '4') {
                if (!distance || (i + j) < distance) {
                    result = LEFT;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }


        if (map[beast->y][beast->x - i] == '1' || map[beast->y][beast->x - i] == '2' ||
            map[beast->y][beast->x - i] == '3' || map[beast->y][beast->x - i] == '4') {
            if (!distance || i < distance) {
                result = LEFT;
                distance = i;
            }
            found_player = 1;
            break;
        }

    }

    found_player = 0, can_check_first_cross = 1, can_check_second_cross = 1;

    for (int i = 1; map[beast->y + i][beast->x] != 'X' && !found_player; ++i) {

        if(map[beast->y+i][beast->x-1] == 'X')
            can_check_first_cross = 0;

        for (int j = 1; map[beast->y + i + j - 1][beast->x - j] != 'X' && can_check_first_cross; ++j) {

            if (map[beast->y + i + j - 1][beast->x - j] == '1' || map[beast->y + i + j - 1][beast->x - j] == '2' ||
                map[beast->y + i + j - 1][beast->x - j] == '3' || map[beast->y + i + j - 1][beast->x - j] == '4') {
                if (!distance || (i + j) < distance) {
                    result = DOWN;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }

        if(map[beast->y+i][beast->x+1])
            can_check_second_cross = 1;


        for (int j = 1; map[beast->y + i + j - 1][beast->x + j] != 'X' && can_check_second_cross; ++j) {

            if (map[beast->y + i + j - 1][beast->x + j] == '1' || map[beast->y + i + j - 1][beast->x + j] == '2' ||
                map[beast->y + i + j - 1][beast->x + j] == '3' || map[beast->y + i + j - 1][beast->x + j] == '4') {
                if (!distance || (i + j) < distance) {
                    result = DOWN;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }


        if (map[beast->y + i][beast->x] == '1' || map[beast->y + i][beast->x] == '2' ||
            map[beast->y + i][beast->x] == '3' || map[beast->y + i][beast->x] == '4') {
            if (!distance || i < distance) {
                result = DOWN;
                distance = i;
            }
            found_player = 1;
            break;
        }
    }

    found_player = 0, can_check_first_cross = 1, can_check_second_cross = 1;

    for (int i = 1; map[beast->y - i][beast->x] != 'X' && !found_player; ++i) {

        if(map[beast->y-i][beast->x-1] == 'X')
            can_check_first_cross = 0;

        for (int j = 1; map[beast->y - i - j + 1][beast->x - j] != 'X' && can_check_first_cross; ++j) {

            if (map[beast->y - i - j + 1][beast->x - j] == '1' || map[beast->y - i - j + 1][beast->x - j] == '2' ||
                map[beast->y - i - j + 1][beast->x - j] == '3' || map[beast->y - i - j + 1][beast->x - j] == '4') {
                if (!distance || (i + j) < distance) {
                    result = UP;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }

        if(map[beast->y-i][beast->x+1] == 'X')
            can_check_second_cross = 0;

        for (int j = 1; map[beast->y - i - j + 1][beast->x + j] != 'X' && can_check_second_cross; ++j) {
            if (map[beast->y - i - j + 1][beast->x + j] == '1' || map[beast->y - i - j + 1][beast->x + j] == '2' ||
                map[beast->y - i - j + 1][beast->x + j] == '3' || map[beast->y - i - j + 1][beast->x + j] == '4') {
                if (!distance || (i + j) < distance) {
                    result = UP;
                    distance = i + j;
                }
                found_player = 1;
                break;
            }
        }

        if (map[beast->y - i][beast->x] == '1' || map[beast->y - i][beast->x] == '2' ||
            map[beast->y - i][beast->x] == '3' || map[beast->y - i][beast->x] == '4') {
            if (!distance || i < distance) {
                result = UP;
                distance = i;
            }
            found_player = 1;
            break;
        }
    }

    return result;
}

int beast_move_up(WINDOW *win, struct beast* beast, struct player** player){
    int result = 0;

    if(map[beast->y][beast->x] == '1' || map[beast->y][beast->x] == '2' || map[beast->y][beast->x] == '3' || map[beast->y][beast->x] == '4'){

        result = (int)(map[beast->y][beast->x] - '0');

        if((*(player+result-1))->carry > 0){
            beast->is_on_something = 1;
            beast->block = 'D';
        }else if((*(player+result-1))->is_on_something){
            beast->is_on_something = 1;
            beast->block = (*(player+result-1))->block;
        }

        map[beast->y][beast->x] = '*';
        return result;
    }

    if(map[beast->y-1][beast->x] != 'X'){

        char c = map[beast->y-1][beast->x];

        if(beast->is_on_something){

            map[beast->y][beast->x] = beast->block;
            beast->is_on_something = 0;
        }else{
            map[beast->y][beast->x] = ' ';
        }

        map[beast->y - 1][beast->x] = '*';
        beast->y--;

        if(c != ' '){

            beast->is_on_something = 1;

            if(c == '1' || c == '2' || c == '3' || c == '4') {
                result = (int)(c - '0');

                c = (*(player+result-1))->block;

                if((*(player+result-1))->carry > 0) {
                    beast->block = 'D';
                }else {
                    if((*(player + result - 1))->is_on_something)
                        beast->block = (*(player + result - 1))->block;
                    else
                        beast->block = ' ';
                }

            }else {
                beast->block = c;
            }

            if(c == '#')
                beast->slow = 1;

        }
    }

    return result;
}

int beast_move_down(WINDOW *win, struct beast* beast, struct player** player){
    int result = 0;

    if(map[beast->y][beast->x] == '1' || map[beast->y][beast->x] == '2' || map[beast->y][beast->x] == '3' || map[beast->y][beast->x] == '4'){
        result = (int)(map[beast->y][beast->x] - '0');

        if((*(player+result-1))->carry > 0){
            beast->is_on_something = 1;
            beast->block = 'D';
        }else if((*(player+result-1))->is_on_something){
            beast->is_on_something = 1;
            beast->block = (*(player+result-1))->block;
        }

        map[beast->y][beast->x] = '*';
        return result;
    }

    if(map[beast->y+1][beast->x] != 'X'){

        char c = map[beast->y+1][beast->x];

        if(beast->is_on_something){
            map[beast->y][beast->x] = beast->block;
            beast->is_on_something = 0;
        }else{
            map[beast->y][beast->x] = ' ';
        }

        map[beast->y + 1][beast->x] = '*';
        beast->y++;

        if(c != ' '){

            beast->is_on_something = 1;

            if(c == '1' || c == '2' || c == '3' || c == '4') {
                result = (int)(c - '0');

                c = (*(player+result-1))->block;

                if((*(player+result-1))->carry > 0) {
                    beast->block = 'D';
                }else {
                    if((*(player + result - 1))->is_on_something)
                        beast->block = (*(player + result - 1))->block;
                    else
                        beast->block = ' ';
                }

            }else {
                beast->block = c;
            }

            if(c == '#')
                beast->slow = 1;

        }
    }
    return result;
}

int beast_move_right(WINDOW *win, struct beast* beast, struct player** player){
    int result = 0;

    if(map[beast->y][beast->x] == '1' || map[beast->y][beast->x] == '2' || map[beast->y][beast->x] == '3' || map[beast->y][beast->x] == '4'){
        result = (int)(map[beast->y][beast->x] - '0');

        if((*(player+result-1))->carry > 0){
            beast->is_on_something = 1;
            beast->block = 'D';
        }else if((*(player+result-1))->is_on_something){
            beast->is_on_something = 1;
            beast->block = (*(player+result-1))->block;
        }

        map[beast->y][beast->x] = '*';
        return result;
    }

    if(map[beast->y][beast->x+1] != 'X'){

        char c = map[beast->y][beast->x+1];

        if(beast->is_on_something){
            map[beast->y][beast->x] = beast->block;
            beast->is_on_something = 0;
        }else{
            map[beast->y][beast->x] = ' ';
        }

        map[beast->y][beast->x+1] = '*';
        beast->x++;

        if(c != ' '){

            beast->is_on_something = 1;

            if(c == '1' || c == '2' || c == '3' || c == '4') {
                result = (int)(c - '0');

                c = (*(player+result-1))->block;

                if((*(player+result-1))->carry > 0) {
                    beast->block = 'D';
                }else {
                    if((*(player + result - 1))->is_on_something)
                        beast->block = (*(player + result - 1))->block;
                    else
                        beast->block = ' ';
                }

            }else {
                beast->block = c;
            }

            if(c == '#')
                beast->slow = 1;

        }
    }

    return result;
}

int beast_move_left(WINDOW *win, struct beast* beast, struct player** player){
    int result = 0;

    if(map[beast->y][beast->x] == '1' || map[beast->y][beast->x] == '2' || map[beast->y][beast->x] == '3' || map[beast->y][beast->x] == '4'){
        result = (int)(map[beast->y][beast->x] - '0');

        if((*(player+result-1))->carry > 0){
            beast->is_on_something = 1;
            beast->block = 'D';
        }else if((*(player+result-1))->is_on_something){
            beast->is_on_something = 1;
            beast->block = (*(player+result-1))->block;
        }

        map[beast->y][beast->x] = '*';
        return result;
    }

    if(map[beast->y][beast->x-1] != 'X'){

        char c = map[beast->y][beast->x-1];

        if(beast->is_on_something){
            map[beast->y][beast->x] = beast->block;
            beast->is_on_something = 0;
        }else{
            map[beast->y][beast->x] = ' ';
        }

        map[beast->y][beast->x-1] = '*';
        beast->x--;

        if(c != ' '){

            beast->is_on_something = 1;

            if(c == '1' || c == '2' || c == '3' || c == '4') {
                result = (int)(c - '0');

                c = (*(player+result-1))->block;

                if((*(player+result-1))->carry > 0) {
                    beast->block = 'D';
                }else {
                    if((*(player + result - 1))->is_on_something)
                        beast->block = (*(player + result - 1))->block;
                    else
                        beast->block = ' ';
                }

            }else {
                beast->block = c;
            }

            if(c == '#')
                beast->slow = 1;

        }
    }

    return result;
}

int beast_movement(WINDOW *win, struct beast* beast, struct player** player){
    int result = 0;

    if(map[beast->y][beast->x] == '1' || map[beast->y][beast->x] == '2' || map[beast->y][beast->x] == '3' || map[beast->y][beast->x] == '4'){
        result = (int)(map[beast->y][beast->x] - '0');

        if((*(player+result-1))->carry > 0){
            beast->is_on_something = 1;
            beast->block = 'D';
        }

        map[beast->y][beast->x] = '*';
        beast->slow = 0;
        return result;
    }else if(beast->slow){
        beast->slow = 0;
        return 0;
    }else {

        enum directions move = find_player(beast);

        int num = rand() % 4;

        if(move != RANDOM)
            num = (int)move;

        switch (num) {
            case 0:
                result = beast_move_up(win, beast, player);
                break;
            case 1:
                result = beast_move_down(win, beast, player);
                break;
            case 2:
                result = beast_move_left(win, beast, player);
                break;
            case 3:
                result = beast_move_right(win, beast, player);
                break;
        }
    }

    return result;
}

void* beast_thread(void* arg){
    if(!arg)
        return NULL;

    struct arg_struct_beast *temp = (struct arg_struct_beast*)arg;

    int res = beast_movement(temp->win, temp->beast, temp->players);

    if(res)
        kill_player(temp->win, *(temp->players+res-1));

    return NULL;
}



void kill_player(WINDOW* win, struct player* player){
    if(player->is_on_something)
        drop_map[player->y][player->x] = player->carry * -1;
    else
        drop_map[player->y][player->x] = player->carry;

    player->carry = 0;

    if(map[player->spawn_y][player->spawn_x] == '*'){
        while(1){
            player->spawn_x = rand()%52;
            player->spawn_y = rand()%25;

            if(map[player->spawn_y][player->spawn_x] == ' ') {
                break;
            }
        }
    }

    player->x = player->spawn_x;
    player->y = player->spawn_y;
    player->is_on_something = 0;
    player->block = ' ';
    player->slow = 0;
    player->deaths++;

    map[player->y][player->x] = player->symbol;
}

int move_up(WINDOW *win, struct player* player){
    if(map[player->y-1][player->x] != 'X') {

        if(map[player->y][player->x] != player->symbol){
            return 1;
        }

        char c = map[player->y - 1][player->x];

        if (player->is_on_something) {
            map[player->y][player->x] = player->block;
            player->is_on_something = 0;

        } else {
            map[player->y][player->x] = ' ';
        }

        player->block = ' ';

        map[player->y - 1][player->x] = player->symbol;
        player->y--;

        if (c == 'c' || c == 't' || c == 'T' || c == 'D') {
            collect_coin(player, c);
        } else if (c == 'A') {
            bank(player);
            player->is_on_something = 1;
            player->block = c;
        } else if (c == '1' || c == '2' || c == '3' || c == '4') {
            player->block = c;
            return 1;
        }else if(c != ' '){
            player->is_on_something = 1;
            player->block = c;

            if (c == '#')
                player->slow = 1;
        }
    }

    return 0;
}

int move_down(WINDOW *win, struct player* player){
    if(map[player->y+1][player->x] != 'X') {

        if(map[player->y][player->x] != player->symbol){
            return 1;
        }

        char c = map[player->y + 1][player->x];

        if (player->is_on_something) {
            map[player->y][player->x] = player->block;
            player->is_on_something = 0;
        } else {
            map[player->y][player->x] = ' ';
        }

        player->block = ' ';

        map[player->y + 1][player->x] = player->symbol;
        player->y++;

        if (c == 'c' || c == 't' || c == 'T' || c == 'D') {
            collect_coin(player, c);
        } else if (c == 'A') {
            bank(player);
            player->is_on_something = 1;
            player->block = c;
        } else if (c == '1' || c == '2' || c == '3' || c == '4') {
            player->block = c;
            return 1;
        }else if(c != ' '){
            player->is_on_something = 1;
            player->block = c;

            if (c == '#')
                player->slow = 1;
        }
    }

    return 0;
}

int move_right(WINDOW *win, struct player* player){
    if(map[player->y][player->x+1] != 'X') {

        if(map[player->y][player->x] != player->symbol){
            return 1;
        }

        char c = map[player->y][player->x + 1];

        if (player->is_on_something) {
            map[player->y][player->x] = player->block;
            player->is_on_something = 0;

        } else {
            map[player->y][player->x] = ' ';
        }

        player->block = ' ';

        map[player->y][player->x + 1] = player->symbol;
        player->x++;

        if (c == 'c' || c == 't' || c == 'T' || c == 'D') {
            collect_coin(player, c);
        } else if (c == 'A') {
            bank(player);
            player->is_on_something = 1;
            player->block = c;
        } else if (c == '1' || c == '2' || c == '3' || c == '4') {
            player->block = c;
            return 1;
        }else if(c != ' '){
            player->is_on_something = 1;
            player->block = c;

            if (c == '#')
                player->slow = 1;
        }
    }

    return 0;
}

int move_left(WINDOW *win, struct player* player){
    if(map[player->y][player->x-1] != 'X') {

        if(map[player->y][player->x] != player->symbol){
            return 1;
        }

        char c = map[player->y][player->x - 1];

        if (player->is_on_something) {
            map[player->y][player->x] = player->block;
            player->is_on_something = 0;
        } else {
            map[player->y][player->x] = ' ';
        }

        player->block = ' ';

        map[player->y][player->x - 1] = player->symbol;
        player->x--;

        if (c == 'c' || c == 't' || c == 'T' || c == 'D') {
            collect_coin(player, c);
        } else if (c == 'A') {
            bank(player);
            player->is_on_something = 1;
            player->block = c;
        } else if (c == '1' || c == '2' || c == '3' || c == '4') {
            player->block = c;
            return 1;
        }else if(c != ' '){
            player->is_on_something = 1;
            player->block = c;

            if (c == '#')
                player->slow = 1;
        }
    }

    return 0;
}

int player_movement(WINDOW *win, struct player* player){
    flushinp();
    int c = wgetch(win);

    player->end_game = tolower((int)c);

    if(player->slow) {
        player->slow = 0;
        return 0;
    }

    int res = 0;

    switch(c){
        case KEY_UP:
            res = move_up(win, player);
            break;
        case KEY_DOWN:
            res = move_down(win, player);
            break;
        case KEY_RIGHT:
            res = move_right(win, player);
            break;
        case KEY_LEFT:
            res = move_left(win, player);
            break;
        case 'c':
            spawn_treasure(win, c);
            break;
        case 't':
            spawn_treasure(win, c);
            break;
        case 'T':
            spawn_treasure(win, c);
            break;
        case 'q':
            break;
        case 'Q':
            break;
        default:
            break;
    }

    return res;
}

void* player_thread(void *arg){
    if(!arg)
        return NULL;

    struct arg_struct_player *temp = (struct arg_struct_player*)arg;

    int res = player_movement(temp->win, temp->player);

    if(res){
        int drop = (*(temp->players))->carry + (*(temp->players+1))->carry;

        if((*(temp->players))->is_on_something && (*(temp->players))->block != '2'){
            drop *= -1;
        }else if((*(temp->players+1))->is_on_something && (*(temp->players+1))->block != '1'){
            drop *= -1;
        }

        if(drop != 0) {
            drop_map[temp->player->y][temp->player->x] = drop;
            map[temp->player->y][temp->player->x] = 'D';
        }else{
            if((*(temp->players))->is_on_something && (*(temp->players))->block != '2'){
                map[(*(temp->players))->y][(*(temp->players))->x] = (*(temp->players))->block;
            }else if((*(temp->players+1))->is_on_something && (*(temp->players+1))->block != '1'){
                map[(*(temp->players+1))->y][(*(temp->players+1))->x] = (*(temp->players+1))->block;
            }else {
                map[temp->player->y][temp->player->x] = ' ';
            }
        }

        temp->player->x = temp->player->spawn_x;
        temp->player->y = temp->player->spawn_y;
        temp->player->carry = 0;
        temp->player->deaths++;
        temp->player->slow = 0;
        temp->player->is_on_something = 0;
        temp->player->block = ' ';

        (*(temp->players+1))->x = (*(temp->players+1))->spawn_x;
        (*(temp->players+1))->y = (*(temp->players+1))->spawn_y;
        (*(temp->players+1))->carry = 0;
        (*(temp->players+1))->deaths++;
        (*(temp->players+1))->slow = 0;
        (*(temp->players+1))->is_on_something = 0;
        (*(temp->players+1))->block = ' ';

        map[temp->player->y][temp->player->x] = temp->player->symbol;
        map[(*(temp->players+1))->y][(*(temp->players+1))->x] = (*(temp->players+1))->symbol;
    }

    return NULL;
}

void* client_thread(void *arg){
    if(!arg)
        return NULL;

    struct arg_struct_client *client = (struct arg_struct_client*)arg;

    int input;

    while(read(client->new_socket, &input, sizeof(int)) <= 0){ }

    if(client->player->slow) {
        if(tolower(input) == (int)'q'){
            if(client->player->carry > 0) {

                if(client->player->is_on_something)
                    drop_map[client->player->y][client->player->x] = client->player->carry * (-1);
                else
                    drop_map[client->player->y][client->player->x] = client->player->carry;

                map[client->player->y][client->player->x] = 'D';
            }else {
                map[client->player->y][client->player->x] = client->player->block;
            }

            (*client->number_of_players)--;
            close(client->new_socket);
            free(client->player);
            *(client->players + *(client->number_of_players)) = NULL;
        }else{
            client->player->slow = 0;
        }
        return NULL;
    }

    int res = 0;

    if(input == KEY_UP){
        res = move_up(client->win, client->player);
    }else if(input == KEY_DOWN){
        res = move_down(client->win, client->player);
    }else if(input == KEY_RIGHT){
        res = move_right(client->win, client->player);
    }else if(input == KEY_LEFT){
        res = move_left(client->win, client->player);
    }else if(tolower(input) == (int)'q'){

        if(client->player->carry > 0) {

            if(client->player->is_on_something)
                drop_map[client->player->y][client->player->x] = client->player->carry * (-1);
            else
                drop_map[client->player->y][client->player->x] = client->player->carry;

            map[client->player->y][client->player->x] = 'D';
        }else {
            map[client->player->y][client->player->x] = client->player->block;
        }

        (*client->number_of_players)--;
        close(client->new_socket);
        free(client->player);
        *(client->players + *(client->number_of_players)) = NULL;
        return NULL;
    }

    if(res){
        int drop = (*(client->players))->carry + (*(client->players+1))->carry;

        if((*(client->players))->is_on_something && (*(client->players))->block != '2'){
            drop *= -1;
        }else if((*(client->players+1))->is_on_something && (*(client->players+1))->block != '1'){
            drop *= -1;
        }

        if(drop != 0) {
            drop_map[client->player->y][client->player->x] = drop;
            map[client->player->y][client->player->x] = 'D';
        }else{
            if((*(client->players))->is_on_something && (*(client->players))->block != '2'){
                map[(*(client->players))->y][(*(client->players))->x] = (*(client->players))->block;
            }else if((*(client->players+1))->is_on_something && (*(client->players+1))->block != '1'){
                map[client->player->y][client->player->x] = client->player->block;
            }else {
                map[client->player->y][client->player->x] = ' ';
            }
        }

        client->player->x = client->player->spawn_x;
        client->player->y = client->player->spawn_y;
        client->player->carry = 0;
        client->player->deaths++;
        client->player->slow = 0;
        client->player->is_on_something = 0;
        client->player->block = ' ';

        (*(client->players))->x = (*(client->players))->spawn_x;
        (*(client->players))->y = (*(client->players))->spawn_y;
        (*(client->players))->carry = 0;
        (*(client->players))->deaths++;
        (*(client->players))->slow = 0;
        (*(client->players))->is_on_something = 0;
        (*(client->players))->block = ' ';

        map[client->player->y][client->player->x] = client->player->symbol;
        map[(*(client->players))->y][(*(client->players))->x] = (*(client->players))->symbol;
    }

    return NULL;
}

void* client_send_map(void *arg){

    struct arg_struct_client *temp = (struct arg_struct_client*)arg;


    int x = temp->player->x - 2;
    int y = temp->player->y - 2;


    if (x <= 0)
        x = 0;
    else if (x > 46)
        x = 46;

    if (y <= 0)
        y = 0;
    else if (y > 20)
        y = 20;


    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (map[y + i][x + j] == 'A') {
                temp->camp_x = x + j;
                temp->camp_y = y + i;
            }
        }
    }


    if(*temp->number_of_players > 1) {

        char player_map[5][5];

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                player_map[i][j] = map[i+y][j+x];
            }
        }
        for (int i = 0; i < 5; ++i) {
            send(temp->new_socket, player_map[i], 5, 0);
        }

        send(temp->new_socket, &temp->counter, sizeof(int), 0);
        send(temp->new_socket, &temp->player->x, sizeof(int), 0);
        send(temp->new_socket, &temp->player->y, sizeof(int), 0);
        send(temp->new_socket, &temp->player->carry, sizeof(int), 0);
        send(temp->new_socket, &temp->player->bank, sizeof(int), 0);
        send(temp->new_socket, &temp->player->deaths, sizeof(int), 0);
        send(temp->new_socket, &temp->camp_x, sizeof(int), 0);
        send(temp->new_socket, &temp->camp_y, sizeof(int), 0);
    }

    return NULL;
}

struct player** create_players(int num){
    struct player **gracze = (struct player**) calloc(4, sizeof(struct player*));

    if(!gracze){
        printf("Not enough memory");
        return NULL;
    }

    for(int i = 0; i < num; ++i){
        *(gracze + i) = add_player();

        if(!*(gracze + i)){

            for(int j = 0; j < i; ++j){
                free(*(gracze+j));
                free(gracze);
            }

            printf("Not enough memory");
            endwin();
            return NULL;
        }

        (*(gracze + i))-> symbol = (char)('0'+i+1);

        map[(*(gracze + i))->y][(*(gracze + i))->x] = (*(gracze + i))->symbol;
    }

    return gracze;
}

int get_maps(){
    FILE *f = fopen("map.txt", "rt");

    if(!f){
        printf("Failed to load the map\n");
        return -1;
    }

    map = (char **) calloc(25, sizeof(char*));

    if(!map){
        printf("Error");
        fclose(f);
        return -2;
    }

    for(int i = 0; i < 25; ++i){
        *(map + i) = (char*) calloc(52, sizeof(char));

        if(!(*(map + i))){

            for(int j = 0; j < i; ++j){
                free(*(map+j));
            }

            printf("Error");
            fclose(f);
            return -2;
        }
    }

    drop_map = (int**) calloc(25, sizeof(int*));

    if(!drop_map){

        for(int i = 0; i < 25; ++i){
            free(*(map+i));
        }
        free(map);
        printf("Error");
        fclose(f);
        return -2;
    }

    for(int i = 0; i < 25; ++i){
        *(drop_map + i ) = (int*) calloc(52, sizeof(int));

        if(!*(drop_map + i)){
            for(int j = 0; j < 25; ++j){
                free(*(map+j));
            }
            free(map);

            for(int j = 0; j < i; ++j){
                free(*(drop_map+j));
            }

            printf("Error");
            fclose(f);
            return -2;
        }
    }

    for(int i = 0; i < 25; ++i){
        for(int j = 0; j < 52; j++){
            char c;

            if(fscanf(f, "%c", &c) != EOF)
                *(*(map + i) + j) = c;
            else{
                printf("Failed to load the map\n");
                fclose(f);
                return -1;
            }
        }
    }

    fclose(f);
    return 0;
}

void free_maps(struct player **players, struct beast *beast, struct arg_struct_player* arg_player, struct arg_struct_beast *arg_beast){
    for(int i = 0; i < 25; ++i){
        free(*(map+i));
        free(*(drop_map+i));
    }

    free(map);
    free(drop_map);

    if(players) {
        for (int i = 0; (*(players + i)) != NULL; ++i) {
            free(*(players + i));
        }
        free(players);
    }

    if(beast)
        free(beast);

    if(arg_player)
        free(arg_player);

    if(arg_beast)
        free(arg_beast);
}
