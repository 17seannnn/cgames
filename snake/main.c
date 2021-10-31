#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#include "config.h"

const char help_text[] = "";
const char version_text[] = "";
const char win_text[] = "";

enum {
        key_escape = 27,

        map_height = 16,
        map_width  = 50,

        delay_duration = 100,

        hor_symb   = '-',
        ver_symb   = '|',
        apple_symb = '@'
};

struct map {
        int min_x, max_x, min_y, max_y;
};

struct tail {
        int cur_x, cur_y, dx, dy;
        struct tail *prev;
};

struct apple {
        int cur_x, cur_y;
};

int rrand(int max)
{
        return (int)((double)max*rand()/(RAND_MAX+1.0));
}

void show_help()
{
        printf("%s", help_text);
}

void show_version()
{
        printf("%s", version_text);
}

int handle_opt(char **argv)
{
        return 1;
}

void initcurses()
{
        initscr();
        if (has_colors())
                start_color();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, 1);
}

void initmap(struct map *m)
{
        int y, x;
        getmaxyx(stdscr, y, x);
        m->min_x = (x - map_width) / 2;
        m->max_x = m->min_x + map_width - 1;
        m->min_y = (y - map_height) / 2;
        m->max_y = m->min_y + map_height - 1;
}

void add_tail(struct tail **s, int x, int y)
{
        struct tail *t = malloc(sizeof(*t));
        t->cur_x = x;
        t->cur_y = y;
        t->prev = *s;
        if (t->prev) {
                t->dx = t->prev->dx;
                t->dy = t->prev->dy;
        } else {
                t->dx = 0;
                t->dy = 0;
        }
        *s = t;
}

void initsnake(struct tail **s, struct map m)
{
        struct tail *t;
        while (*s) {
                t = *s;
                s = &(*s)->prev;
                free(t);
        }
        *s = NULL;
        add_tail(s, (m.min_x + m.max_x) / 2, (m.min_y + m.max_y) / 2);
}

int is_empty(int x, int y, struct tail *s)
{
        for (; s; s = s->prev)
                if (x == s->cur_x && y == s->cur_y)
                        return 0;
        return 1;
}

void initapple(struct apple *a, struct tail *s, struct map m)
{
        int x, y;
        do {
                x = m.min_x + rrand(map_width-2) + 1;
                y = m.min_y + rrand(map_height-2) + 1;
        } while (!is_empty(x, y, s));
        a->cur_x = x;
        a->cur_y = y;
}

void initgame(struct map *m, struct tail **s, struct apple *a, int *score)
{
        srand(time(NULL));
        timeout(delay_duration);
        *score = 1;
        initmap(m);
        initsnake(s, *m);
        initapple(a, *s, *m);
}

void check(int *coord, int min, int max)
{
        if (*coord < min)
                *coord = max;
        if (*coord > max)
                *coord = min;
}

int didwin(struct map m, struct tail *s)
{
        int x, y;
        for (y = m.min_y + 1; y < m.max_y; y++)
                for (x = m.min_x + 1; x < m.max_x; x++)
                        if (is_empty(x, y, s))
                                return 0;
        return 1;
}

void show_map(struct map m)
{
        int x, y;
        for (y = m.min_y; y <= m.max_y; y++) {
                if (y == m.min_y || y == m.max_y) {
                        for (x = m.min_x; x <= m.max_x; x++)
                                mvaddch(y, x, hor_symb);
                } else {
                        mvaddch(y, m.min_x, ver_symb);
                        mvaddch(y, m.max_x, ver_symb);
                }
        }
        refresh();
}

void show_snake(struct tail *s)
{
        for (; s; s = s->prev)
                mvaddch(s->cur_y, s->cur_x, s->dx ? hor_symb : ver_symb);
        refresh();
}

void hide_snake(struct tail *s)
{
        for (; s; s = s->prev)
                mvaddch(s->cur_y, s->cur_x, ' ');
        refresh();
}

void set_direction(struct tail *s, int dx, int dy)
{
        if ((s->dx != 0 && s->dx == -dx) || (s->dy != 0 && s->dy == -dy))
                return;
        if (s->prev)
                set_direction(s->prev, s->dx, s->dy);
        s->dx = dx;
        s->dy = dy;
}

void set_coords(struct tail *s, int x, int y, struct map m)
{
        if (s->prev)
                set_coords(s->prev, s->cur_x, s->cur_y, m);
        s->cur_x = x;
        s->cur_y = y;
        check(&s->cur_x, m.min_x+1, m.max_x-1);
        check(&s->cur_y, m.min_y+1, m.max_y-1);
}

void move_snake(struct tail *s, struct map m)
{
        hide_snake(s);
        set_coords(s, s->cur_x + s->dx, s->cur_y + s->dy, m);
        show_snake(s);
}

void show_apple(struct apple a)
{
        mvaddch(a.cur_y, a.cur_x, apple_symb);
        refresh();
}

void hide_apple(struct apple a)
{
        mvaddch(a.cur_y, a.cur_x, ' ');
        refresh();
}

int move_apple(struct apple *a, struct tail *s, struct map m)
{
        if (didwin(m, s))
                return 0;
        initapple(a, s, m);
        show_apple(*a);
        return 1;
}

int check_collision(struct tail *s, struct apple a)
{
        if (s->cur_x + s->dx == a.cur_x && s->cur_y + s->dy == a.cur_y)
                return -1;
        if (0 == is_empty(s->cur_x + s->dx, s->cur_y + s->dy, s->prev))
                return 0;
        return 1;
}

void show_score(int score)
{
        mvprintw(0, 0, "%d", score);
        refresh();
}

void draw_screen(struct map m, struct tail *s, struct apple a)
{
        clear();
        show_map(m);
        show_snake(s);
        show_apple(a);
}

void handle_resize(struct map *m, struct tail *s, struct apple *a)
{
        int diff_x, diff_y;
        struct map old = *m;
        initmap(m);
        diff_x = m->min_x - old.min_x;
        diff_y = m->min_y - old.min_y;
        for (; s; s = s->prev) {
                s->cur_x += diff_x;
                s->cur_y += diff_y;
        }
        a->cur_x += diff_x;
        a->cur_y += diff_y;
        draw_screen(*m, s, *a);
}

int playgame(struct map *m, struct tail **s, struct apple *a, int *score)
{
        int key, res;
        while ((key = getch()) != key_escape) {
                show_score(*score);
                switch (key) {
                case KEY_UP:
                case 'W': case 'w':
                case 'K': case 'k':
                        set_direction(*s, 0, -1);
                        break;
                case KEY_DOWN:
                case 'S': case 's':
                case 'J': case 'j':
                        set_direction(*s, 0, 1);
                        break;
                case KEY_LEFT:
                case 'A': case 'a':
                case 'H': case 'h':
                        set_direction(*s, -1, 0);
                        break;
                case KEY_RIGHT:
                case 'D': case 'd':
                case 'L': case 'l':
                        set_direction(*s, 1, 0);
                        break;
                case KEY_RESIZE:
                        handle_resize(m, *s, a);
                        break;
                default:
                        set_direction(*s, (*s)->dx, (*s)->dy);
                }
                res = check_collision(*s, *a);
                if (res < 0) {
                        add_tail(s, a->cur_x, a->cur_y);
                        ++*score;
                        if (0 == move_apple(a, *s, *m))
                                return 1;
                } else if (!res) {
                        break;
                }
                move_snake(*s, *m);
        }
        return 0;
}

void endgame(int win, int score)
{
        int x, y;
        clear();
        getmaxyx(stdscr, y, x);
        y /= 2;
        x = (x - strlen(win_text))/2;
        if (win)
                mvprintw(y, x, "%s", win_text);
        else
                mvprintw(y, x, "%d", score);
        refresh();
        sleep(3);
}

/* TODO */
int ask_continue()
{
        clear();
        return 0;
}

int main(int argc, char **argv)
{
        int win, score;
        struct map m;
        struct tail *s = NULL;
        struct apple a;
        if (!handle_opt(argv))
                return 0;
        initcurses();
        do {
                initgame(&m, &s, &a, &score);
                draw_screen(m, s, a);
                win = playgame(&m, &s, &a, &score);
                endgame(win, score);
        } while (ask_continue());
        endwin();
        return 0;
}
