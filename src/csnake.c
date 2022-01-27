#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#include "cgames.h"

#define _(STR) gettext(STR)
#define gettext_noop(STR) STR
#define N_(STR) (STR)

static const char win_text[] = gettext_noop("You win!");
static const char continue_text[] = gettext_noop("Continue? [y/N]");
static const char endgame_text[] = gettext_noop("Steps: %d   Score: %d");

enum {
        buffer_size = 1024,

        key_escape = 27,

        map_height = 16,
        map_width  = 50,
        max_score  = (map_height - 2) * (map_width - 2),

        delay_duration = 100,

        standing_symb = '.',
        left_symb     = '<',
        right_symb    = '>',
        up_symb       = '^',
        down_symb     = 'v',
        hor_symb      = '-',
        ver_symb      = '|',
        apple_symb    = '@',
        bonus_symb    = '?',

        bonus_chance = 5,
        bonus_end = -1,
        bonus_off = 0,
        bonus_on  = 1,

        min_steps   = 50,
        range_steps = 50,

        map_pair = 1,
        snake_pair,
        apple_pair,
        bonus_pair,

        bg_color    = COLOR_BLACK,
        map_color   = COLOR_CYAN,
        snake_color = COLOR_GREEN,
        apple_color = COLOR_RED,
        bonus_color = COLOR_YELLOW

};

struct map {
        int min_x, max_x, min_y, max_y;
};

struct tail {
        int cur_x, cur_y, dx, dy;
        int steps, score;
        struct tail *prev;
};

struct apple {
        int cur_x, cur_y;
};

struct bonus {
        int cur_x, cur_y, steps, status;
        double effect;
};

static int rrand(int max)
{
        return (int)((double)max*rand()/(RAND_MAX+1.0));
}

static void initcolors()
{
        init_pair(map_pair, map_color, bg_color);
        init_pair(snake_pair, snake_color, bg_color);
        init_pair(apple_pair, apple_color, bg_color);
        init_pair(bonus_pair, bonus_color, bg_color);
}

static void initcurses()
{
        initscr();
        if (has_colors())
                start_color();
        initcolors();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, 1);
}

static int is_tail(int x, int y, struct tail *s)
{
        for (; s; s = s->prev)
                if (x == s->cur_x && y == s->cur_y)
                        return 1;
        return 0;
}

static int is_apple(int x, int y, struct apple a)
{
        if (x == a.cur_x && y == a.cur_y)
                return 1;
        return 0;
}

static int is_bonus(int x, int y, struct bonus b)
{
        if (x == b.cur_x && y == b.cur_y)
                return 1;
        return 0;
}

static int has_empty(struct map m, struct tail *s, struct apple a, 
                                                   struct bonus b)
{
        int x, y;
        for (y = m.min_y + 1; y < m.max_y; y++)
                for (x = m.min_x + 1; x < m.max_x; x++)
                        if (!is_tail(x, y, s)  &&
                            !is_apple(x, y, a) &&
                            !is_bonus(x, y, b))
                                return 1;
        return 0;
}

static void check(int *coord, int min, int max)
{
        if (*coord < min)
                *coord = max;
        if (*coord > max)
                *coord = min;
}

static void initmap(struct map *m)
{
        int y, x;
        getmaxyx(stdscr, y, x);
        m->min_x = (x - map_width) / 2;
        m->max_x = m->min_x + map_width - 1;
        m->min_y = (y - map_height) / 2;
        m->max_y = m->min_y + map_height - 1;
}

static void add_tail(struct tail **s, struct map m)
{
        struct tail *t = malloc(sizeof(*t));
        if (*s) {
                for (; (*s)->prev ; s = &(*s)->prev)
                        {}
                (*s)->prev = t;
                t->cur_x = (*s)->cur_x - (*s)->dx;
                t->cur_y = (*s)->cur_y - (*s)->dy;
                check(&t->cur_x, m.min_x, m.max_x);
                check(&t->cur_y, m.min_y, m.max_y);
                t->dx = (*s)->dx;
                t->dy = (*s)->dy;
        } else {
                *s = t;
                t->cur_x = (m.min_x + m.max_x) / 2;
                t->cur_y = (m.min_y + m.max_y) / 2;
                t->dx = 0;
                t->dy = 0;
                t->steps = 0;
                t->score = 1;
        }
                t->prev = NULL;
}

static void initsnake(struct tail **s, struct map m)
{
        struct tail *t;
        while (*s) {
                t = *s;
                *s = (*s)->prev;
                free(t);
        }
        add_tail(s, m);
}

static void initapple(struct apple *a, struct tail *s, struct map m)
{
        int x, y;
        do {
                x = m.min_x + rrand(map_width-2) + 1;
                y = m.min_y + rrand(map_height-2) + 1;
        } while (is_tail(x, y, s));
        a->cur_x = x;
        a->cur_y = y;
}

static void initbonus(struct bonus *b, struct tail *s, struct apple a,
                                                       struct map m)
{
        int x, y;
        if (!has_empty(m, s, a, *b)) {
                b->cur_x = -1;
                b->cur_x = -1;
                return;
        }
        do {
                x = m.min_x + rrand(map_width-2) + 1;
                y = m.min_y + rrand(map_height-2) + 1;
        } while (is_tail(x, y, s) || is_apple(x, y, a));
        b->cur_x = x;
        b->cur_y = y;
        b->steps = min_steps + rrand(range_steps);
        if (rrand(2))
                b->effect = 1 + 0.25 * (rrand(4) + 1);
        else
                b->effect = 0.25 * (rrand(4) + 1);
        b->status = bonus_off;
}

static void initgame(struct map *m, struct tail **s, struct apple *a,
                                                     struct bonus *b)
{
        srand(time(NULL));
        timeout(delay_duration);
        initmap(m);
        initsnake(s, *m);
        initapple(a, *s, *m);
        initbonus(b, *s, *a, *m);
}

static void show_map(struct map m)
{
        int x, y;
        attron(COLOR_PAIR(map_pair));
        for (y = m.min_y; y <= m.max_y; y++) {
                if (y == m.min_y || y == m.max_y) {
                        for (x = m.min_x; x <= m.max_x; x++)
                                mvaddch(y, x, hor_symb);
                } else {
                        mvaddch(y, m.min_x, ver_symb);
                        mvaddch(y, m.max_x, ver_symb);
                }
        }
        attroff(COLOR_PAIR(map_pair));
        refresh();
}

static void show_snake(struct tail *s)
{
        attron(COLOR_PAIR(snake_pair));
        if (s->dx != 0)
                mvaddch(s->cur_y, s->cur_x, s->dx < 0 ? left_symb : right_symb);
        else
        if (s->dy != 0)
                mvaddch(s->cur_y, s->cur_x, s->dy < 0 ? up_symb : down_symb);
        else
                mvaddch(s->cur_y, s->cur_x, standing_symb);
        for (s = s->prev; s; s = s->prev)
                mvaddch(s->cur_y, s->cur_x, s->dx ? hor_symb : ver_symb);
        attroff(COLOR_PAIR(snake_pair));
        refresh();
}

static void hide_snake(struct tail *s)
{
        for (; s; s = s->prev)
                mvaddch(s->cur_y, s->cur_x, ' ');
        refresh();
}

static void set_direction(struct tail *s, int dx, int dy)
{
        if ((s->dx != 0 && s->dx == -dx) || (s->dy != 0 && s->dy == -dy))
                return;
        if (s->prev)
                set_direction(s->prev, s->dx, s->dy);
        s->dx = dx;
        s->dy = dy;
}

static void set_coords(struct tail *s, int x, int y, struct map m)
{
        if (s->prev)
                set_coords(s->prev, s->cur_x, s->cur_y, m);
        s->cur_x = x;
        s->cur_y = y;
        check(&s->cur_x, m.min_x+1, m.max_x-1);
        check(&s->cur_y, m.min_y+1, m.max_y-1);
}

static void move_snake(struct tail *s, struct map m)
{
        hide_snake(s);
        set_coords(s, s->cur_x + s->dx, s->cur_y + s->dy, m);
        show_snake(s);
        if (s->dx || s->dy)
                s->steps++;
}

static void show_apple(struct apple a)
{
        mvaddch(a.cur_y, a.cur_x, apple_symb | COLOR_PAIR(apple_pair));
        refresh();
}

static void move_apple(struct apple *a, struct tail *s, struct bonus *b,
                                                        struct map m)
{
        initapple(a, s, m);
        show_apple(*a);
}

static void show_bonus(struct bonus b)
{
        mvaddch(b.cur_y, b.cur_x, bonus_symb | COLOR_PAIR(bonus_pair));
        refresh();
}

static void move_bonus(struct bonus *b, struct tail *s, struct apple a,
                                                        struct map m)
{
        initbonus(b, s, a, m);
        show_bonus(*b);
}

static void handle_bonus(struct bonus *b, struct tail *s, struct apple a,
                                                          struct map m)
{
        switch (b->status) {
        case bonus_end:
                timeout(delay_duration);
                if (rrand(100) < bonus_chance)
                        move_bonus(b, s, a, m);
        case bonus_off:
                return;
        case bonus_on:
                timeout((int)(delay_duration * b->effect));
                b->steps--;
                if (b->steps <= 0)
                        b->status = bonus_end;
        }
}

static int check_collision(struct tail *s,
                    struct apple a,
                    struct bonus *b,
                    struct map m)
{
        if (s->cur_x == b->cur_x && s->cur_y == b->cur_y) {
                if (b->status != bonus_end)
                        b->status = bonus_on;
        }
        if (s->cur_x == a.cur_x && s->cur_y == a.cur_y)
                return -1;
        return !is_tail(s->cur_x, s->cur_y, s->prev);
}

static void show_info(int steps, int score)
{
        mvprintw(0, 0, N_("%d %d"), steps, score);
        refresh();
}

static void draw_screen(struct map m, struct tail *s, struct apple a,
                                                      struct bonus b)
{
        clear();
        show_map(m);
        show_snake(s);
        show_apple(a);
        show_bonus(b);
}

static void handle_resize(struct map *m,
                          struct tail *s,
                          struct apple *a,
                          struct bonus *b)
{
        int diff_x, diff_y;
        struct tail *t;
        struct map old = *m;
        initmap(m);
        diff_x = m->min_x - old.min_x;
        diff_y = m->min_y - old.min_y;
        for (t = s; t; t = t->prev) {
                t->cur_x += diff_x;
                t->cur_y += diff_y;
        }
        a->cur_x += diff_x;
        a->cur_y += diff_y;
        b->cur_x += diff_x;
        b->cur_y += diff_y;
        draw_screen(*m, s, *a, *b);
}

static void playgame(struct map *m, struct tail **s, struct apple *a,
                                                     struct bonus *b)
{
        int key, res;
        while ((key = getch()) != key_escape) {
                show_info((*s)->steps, (*s)->score);
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
                        handle_resize(m, *s, a, b);
                        set_direction(*s, (*s)->dx, (*s)->dy);
                        break;
                default:
                        set_direction(*s, (*s)->dx, (*s)->dy);
                }
                handle_bonus(b, *s, *a, *m);
                move_snake(*s, *m);
                res = check_collision(*s, *a, b, *m);
                if (res < 0) {
                        add_tail(s, *m);
                        (*s)->score++;
                        if ((*s)->score >= max_score)
                                return;
                        move_apple(a, *s, b, *m);
                } else if (!res) {
                        sleep(1);
                        return;
                }
        }
}

static void endgame(int steps, int score)
{
        int x, y;
        clear();
        getmaxyx(stdscr, y, x);
        y /= 2;
        if (score >= max_score) {
                x = (x - strlen(_(win_text))) / 2;
                mvprintw(y, x, _(win_text));
        } else {
                x = (x - strlen(_(endgame_text))) / 2;
                mvprintw(y, x, _(endgame_text), steps, score);
        }
        refresh();
}

static int ask_continue()
{
        int y, x, key, ans = 'N';
        getmaxyx(stdscr, y, x);
        mvprintw(y/2 + 1, (x - strlen(_(continue_text))) / 2, _(continue_text));
        refresh();
        timeout(-1);
        y = getcury(stdscr);
        x = getcurx(stdscr) + 1;
        do {
                key = getch();
                if (key == 'Y' || key == 'y' || key == 'N' || key == 'n') {
                        ans = key;
                        mvaddch(y, x, key);
                }
        } while (key != '\n');
        if (ans == 'Y' || ans == 'y')
                return 1;
        return 0;
}

static void freegame(struct tail *s)
{
        struct tail *t;
        while (s) {
                t = s;
                s = s->prev;
                free(t);
        }
}

void csnake()
{
        struct map m;
        struct tail *s = NULL;
        struct apple a;
        struct bonus b;
        initcurses();
        do {
                initgame(&m, &s, &a, &b);
                draw_screen(m, s, a, b);
                playgame(&m, &s, &a, &b);
                endgame(s->steps, s->score);
        } while (ask_continue());
        endwin();
        freegame(s);
}
