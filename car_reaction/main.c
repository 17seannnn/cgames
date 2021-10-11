#include <stdio.h>
#include <curses.h>
#include <unistd.h>

enum {
        key_escape     = 27,
        min_row        = 24,
        min_col        = 80,
        delay_duration = 100,
        car_symbol     = 'I',
        min_pos        = 1,
        max_pos        = 4,
        road_height    = 16,
        road_width     = 11,
        barrier_count  = max_pos
};

struct map {
        int min_x, max_x, min_y, max_y;
};

struct car {
        int cur_x, cur_y, pos, score;
        char symb;
};

struct barrier {
        int cur_x, cur_y, length;
};

int check_screen()
{
        int row, col;
        getmaxyx(stdscr, row, col);
        if (row < min_row || col < min_col)
                return 0;
        else
                return 1;
}

void init_game(struct map *m, struct car *c, struct barrier *b)
{
        int row, col, map_h, map_w;
        getmaxyx(stdscr, row, col);
        map_w = (road_width * max_pos) + (max_pos - 1) + 2;
        map_h = road_height;
        m->min_x = (col - map_w) / 2;
        m->max_x = m->min_x + map_w - 1;
        m->min_y = (row - map_h) / 2;
        m->max_y = m->min_y + map_h - 1;
        c->pos = 2;
        c->cur_x = m->min_x + road_width * c->pos + c->pos-1 - road_width/2;
        c->cur_y = m->max_y - map_h/5;
        c->score = 0;
        c->symb = car_symbol;
}

void handle_resize(struct map *m, struct car *c, struct barrier *b)
{

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
        mvaddch(c.cur_y, c.cur_x, c.symb);
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
