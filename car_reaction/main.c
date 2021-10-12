#include <stdio.h>
#include <stdlib.h>
#include <curses.h>

enum {
        min_row        = 24,
        min_col        = 80,
        key_escape     = 27,
        delay_duration = 150,
        car_symbol     = 'I',
        barrier_symbol = '=',
        start_pos      = 2,
        min_pos        = 1,
        max_pos        = 4,
        road_height    = 16,
        road_width     = 11,
        barrier_count  = max_pos,
        barrier_width  = road_width
};

struct map {
        int min_x, max_x, min_y, max_y, h, w;
};

struct car {
        int cur_x, cur_y, pos, score;
};

struct barrier {
        int cur_x, cur_y, height, step;
};

int check_screen()
{
        int row, col;
        getmaxyx(stdscr, row, col);
        if (row < min_row || col < min_col)
                return 0;
        return 1;
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

void init_barrier(struct barrier *b, int n, int save, struct map m)
{
        int i;
        for (i = 0; i < n; i++) {
                if (save) {
                        b[i].cur_x = m.min_x+1 + road_width*i;
                        b[i].cur_y = m.min_y + b[i].step;
                } else {
                        b[i].cur_x = m.min_x+1 + road_width*i;
                        b[i].cur_y = m.min_y - rand() % 10;
                        b[i].height = 1;
                        b[i].step = b[i].cur_y - m.min_y;
                }
        }
}

void init_game(struct map *m, struct car *c, struct barrier *b)
{
        init_map(m);
        init_car(c, start_pos, 0, *m);
        init_barrier(b, barrier_count, 0, *m);
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
        mvaddch(c.cur_y, c.cur_x, car_symbol);
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

void draw_screen(struct map m, struct car c, struct barrier *b)
{
        clear();
        show_map(m);
        show_road(m, c.score);
        show_car(c);
        /* show_barrier(b); */
}

void handle_resize(struct map *m, struct car *c, struct barrier *b)
{
        init_map(m);
        init_car(c, c->pos, c->score, *m);
        init_barrier(b, barrier_count, 0, *m);
        draw_screen(*m, *c, b);
}

int main()
{
        int key;
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
        timeout(delay_duration);
        if (!check_screen()) {
                endwin();
                fprintf(stderr, "Resize your window to %dx%d\n",
                                min_row, min_col);
                return 1;
        }
        init_game(&m, &c, b);
        draw_screen(m, c, b);
        while ((key = getch()) != key_escape) {
                mvprintw(0, 0, "%d", c.score);
                show_road(m, c.score);
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
                        handle_resize(&m, &c, b);
                        break;
                }
                c.score++;
        }
        endwin();
        return 0;
}
