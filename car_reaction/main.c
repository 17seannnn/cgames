#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>

enum {
        min_row            = 24,
        min_col            = 80,
        key_escape         = 27,
        delay_duration     = 50,
        car_symbol         = 'I',
        barrier_symbol     = '=',
        start_pos          = 2,
        min_pos            = 1,
        max_pos            = 3,
        road_height        = 16,
        road_width         = 11,
        barrier_count      = max_pos,
        barrier_width      = road_width,
        max_score          = 32767,
        min_speed          = 50,
        max_speed          = 75,
        car_pair           = 1,
        barrier_pair
};

struct map {
        int min_x, max_x, min_y, max_y, h, w;
};

struct car {
        int cur_x, cur_y, pos, score;
};

struct barrier {
        int cur_x, cur_y, step;
};

int rrand(int max)
{
        return (int)((double)max*rand()/(RAND_MAX+1.0));
}

int check_screen()
{
        int row, col;
        getmaxyx(stdscr, row, col);
        if (row < min_row || col < min_col)
                return 0;
        return 1;
}

void init_colors()
{
        init_pair(car_pair, COLOR_BLUE, COLOR_BLACK);
        init_pair(barrier_pair, COLOR_RED, COLOR_BLACK);
}

void init_map(struct map *m)
{
        int row, col;
        getmaxyx(stdscr, row, col);
        m->w = (road_width * max_pos) + (max_pos - 1) + 2;
        m->h = road_height;
        m->min_x = (col - m->w) / 2;
        m->max_x = m->min_x + m->w - 1;
        m->min_y = (row - m->h) / 2;
        m->max_y = m->min_y + m->h - 1;
}

void init_car(struct car *c, int pos, int score, struct map m)
{
        c->cur_x = m.min_x + road_width * pos + pos-1 - road_width/2;
        c->cur_y = m.max_y - m.h/5;
        c->pos = pos;
        c->score = score;
}

void check_barrier(struct barrier *b, int n)
{
        int i;
        for (i = 0; i < n; i++) {
                if (i > 0)
                        if (b[i].cur_y   != b[i-1].cur_y &&
                            b[i].cur_y-1 != b[i-1].cur_y &&
                            b[i].cur_y+1 != b[i-1].cur_y)
                                return;
                if (i+1 != n)
                        if (b[i].cur_y   != b[i+1].cur_y &&
                            b[i].cur_y-1 != b[i+1].cur_y &&
                            b[i].cur_y+1 != b[i+1].cur_y)
                                return;
        }
        b[0].cur_y -= 3;
        b[0].step -= 3;
}

void init_barrier(struct barrier *b, int n, int save, struct map m)
{
        if (save) {
                b->cur_x = m.min_x+1 + road_width*n;
                b->cur_y = m.min_y + b->step;
        } else {
                b->cur_x = m.min_x+1 + road_width*n + n;
                b->step = rrand(-10);
                b->cur_y = m.min_y + b->step;
        }
}

void init_game(struct map *m, struct car *c, struct barrier *b, int n)
{
        int i;
        init_colors();
        init_map(m);
        init_car(c, start_pos, 0, *m);
        for (i = 0; i < n; i++)
                init_barrier(&b[i], i, 0, *m);
        check_barrier(b, n);
}

void show_map(struct map m)
{
        int y;
        for (y = m.min_y; y <= m.max_y; y++) {
                mvaddch(y, m.min_x, '|');
                mvaddch(y, m.max_x, '|');
        }
        refresh();
}

void show_score(int score, int y, int x)
{
        mvprintw(y, x, "%d", score);
}

void show_road(struct map m, int score)
{
        int x, y, pos;
        for (y = m.min_y; y <= m.max_y; y++) {
                x = m.min_x;
                for (pos = min_pos; pos <= max_pos-1; pos++) {
                        x += road_width + 1;
                        mvaddch(y, x, (y + score) % 2 == 0 ? '|' : ' ');
                }
        }
        refresh();
}

void show_car(struct car c)
{
        mvaddch(c.cur_y, c.cur_x, car_symbol | COLOR_PAIR(car_pair));
        refresh();
}

void hide_car(struct car c)
{
        mvaddch(c.cur_y, c.cur_x, ' ');
        refresh();
}

void move_car(struct car *c, int dpos)
{
        int pos;
        hide_car(*c);
        pos = c->pos + dpos;
        if (pos < min_pos)
                pos = min_pos;
        else if (pos > max_pos)
                pos = max_pos;
        if (c->pos < pos)
                c->cur_x += road_width + 1; 
        if (c->pos > pos)
                c->cur_x -= road_width + 1; 
        c->pos = pos;
        show_car(*c);
}

void show_barrier(struct barrier *b, int n)
{
        int i, j;
        for (i = 0; i < n; i++) {
                if (b[i].step >= 0) {
                        move(b[i].cur_y, b[i].cur_x);
                        for (j = 0; j < road_width; j++)
                                addch('=' | COLOR_PAIR(barrier_pair));
                }
        }
        refresh();
}

void hide_barrier(struct barrier *b, int n)
{
        int i, j;
        for (i = 0; i < n; i++) {
                if (b[i].step >= 0) {
                        move(b[i].cur_y, b[i].cur_x);
                        for (j = 0; j < road_width; j++)
                                addch(' ');
                }
        }
        refresh();
}

void move_barrier(struct barrier *b, int n, struct map m)
{
        int i;
        hide_barrier(b, n);
        for (i = 0; i < n; i++) {
                if (b[i].step < road_height) {
                        b[i].cur_y++;
                        b[i].step++;
                } else {
                        init_barrier(&b[i], i, 0, m);
                }
        }
        check_barrier(b, n);
        show_barrier(b, n);
}

int check_collision(struct car c, struct barrier *b, int n)
{
        int i;
        for (i = 0; i < n; i++)
                if ((c.cur_y   == b[i].cur_y ||
                     c.cur_y-1 == b[i].cur_y) &&
                    (c.cur_x > b[i].cur_x &&
                     c.cur_x < b[i].cur_x + road_width))
                                return 1;
        return 0;
}

void speedup(int score)
{
        int speed = max_speed - 5 * (score / (max_score/100));
        if (speed < min_speed)
                speed = min_speed;
        refresh();
        timeout(speed);
}

void draw_screen(struct map m, struct car c, struct barrier *b, int n)
{
        clear();
        show_map(m);
        show_road(m, c.score);
        show_car(c);
        show_barrier(b, n);
}

void handle_resize(struct map *m, struct car *c, struct barrier *b, int n)
{
        int i;
        init_map(m);
        init_car(c, c->pos, c->score, *m);
        for (i = 0; i < n; i++)
                init_barrier(&b[i], i, 0, *m);
        check_barrier(b, n);
        draw_screen(*m, *c, b, n);
}

void endgame(int score)
{
        int x, y;
        getmaxyx(stdscr, y, x);
        sleep(1);
        clear();
        if (score < max_score)
                show_score(score, y/2, (x-5)/2);
        else
                mvprintw(y/2, (x-10)/2, "You win!");
        mvprintw(y-1, 0, "Press any key to exit");
        refresh();
        sleep(1);
        timeout(-1);
        getch();
}

int main()
{
        int res, key;
        struct map m;
        struct car c;
        struct barrier b[barrier_count];
        initscr();
        if (has_colors())
                start_color();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, 1);
        timeout(min_speed);
        srand(time(NULL));
        if (!check_screen()) {
                endwin();
                fprintf(stderr, "Resize your window to %dx%d\n",
                                min_row, min_col);
                return 1;
        }
        init_game(&m, &c, b, barrier_count);
        draw_screen(m, c, b, barrier_count);
        while ((key = getch()) != key_escape) {
                show_score(c.score, 0, 0);
                switch (key) {
                case KEY_LEFT:
                case 'A':
                case 'a':
                        move_car(&c, -1);
                        break;
                case KEY_RIGHT:
                case 'D':
                case 'd':
                        move_car(&c, 1);
                        break;
                case KEY_RESIZE:
                        handle_resize(&m, &c, b, barrier_count);
                        break;
                }
                c.score++;
                show_road(m, c.score);
                res = check_collision(c, b, barrier_count);
                if (res)
                        break;
                if (c.score >= max_score)
                        break;
                move_barrier(b, barrier_count, m);
                speedup(c.score);
        }
        endgame(c.score);
        endwin();
        return 0;
}
