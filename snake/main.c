#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>

#include "config.h"

const char help_text[] = "";
const char version_text[] = "";

enum {
        map_height = 16,
        map_width  = 50,

        hor_symb   = '-',
        ver_symb   = '|',

        apple_symb = '@'
};

struct map {
        int min_x, max_x, min_y, max_y;
};

struct tail {
        int cur_x, cur_y, dx, dy;
        struct tail *next;
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

void initsnake(struct tail **s, struct map m)
{
        struct tail *t;
        for (t = *s; *s; s = &(*s)->next)
                free(t);
        *s = malloc(sizeof(**s));
        (*s)->cur_x = (m.min_x + m.max_x) / 2;
        (*s)->cur_y = (m.min_y + m.max_y) / 2;
        (*s)->dx = 0;
        (*s)->dy = 0;
        (*s)->next = NULL;
}

int is_empty(int x, int y, struct tail *s)
{
        for (; s; s = s->next)
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
}

void initgame(struct map *m, struct tail **s, struct apple *a)
{
        initmap(m);
        initsnake(s, *m);
        initapple(a, *s, *m);
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
        for (; s; s = s->next)
                mvaddch(s->cur_y, s->cur_x, s->dx ? hor_symb : ver_symb);
}

void hide_snake(struct tail *s)
{
        for (; s; s = s->next)
                mvaddch(s->cur_y, s->cur_x, ' ');
}

void show_apple(struct apple a)
{
        mvaddch(a.cur_y, a.cur_x, apple_symb);
}

void hide_apple(struct apple a)
{
        mvaddch(a.cur_y, a.cur_x, ' ');
}

void draw_screen(struct map m, struct tail *s, struct apple a)
{
        show_map(m);
        show_snake(s);
        show_apple(a);
}

void playgame(struct map m, struct tail *s, struct apple *a)
{

}

int ask_continue()
{
        return 0;
}

int main(int argc, char **argv)
{
        struct map m;
        struct tail *s = NULL;
        struct apple a;
        if (!handle_opt(argv))
                return 0;
        initcurses();
        do {
                initgame(&m, &s, &a);
                draw_screen(m, s, a);
                sleep(2);
                playgame(m, s, &a);
        } while (ask_continue());
        endwin();
        return 0;
}
