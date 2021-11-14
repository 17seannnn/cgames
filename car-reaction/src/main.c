#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#include "config.h"

#define LOCALEDIR "/usr/share/locale"
#define LOCAL_LOCALEDIR "/.local/share/locale"

#define _(STR) gettext(STR)
#define gettext_noop(STR) STR
#define N_(STR) STR

#define PROGRAM_NAME "cr"
#define PACKAGE_NAME "cgames"
#define PACKAGE_PAGE "https://github.com/17seannnn/cgames"
#define VERSION "1.70"

#define LICENSE "GPLv3"
#define LICENSE_PAGE "https://gnu.org/licenses/gpl.html"
#define COPYRIGHT_YEAR "2021"

#define AUTHOR "Sergey S. Nikonov"
#define AUTHOR_NICKNAME "17seannnn"
#define AUTHOR_PAGE "https://github.com/17seannnn"

#define HELP_SHORT_OPT "-h"
#define HELP_LONG_OPT "--help"
#define VERSION_SHORT_OPT "-v"
#define VERSION_LONG_OPT "--version"

const char help_text[] = gettext_noop("\
Usage: %s [-OPT/--OPT]\n\
\n\
Options\n\
\n\
-h, --help      show help\n\
-v, --version   show version\n\
\n\
Movement\n\
\n\
'left arrow' and 'right arrow' or\n\
'A','a' and 'D', 'd'\n\
\n\
Report bugs to & %s home page: <%s>\n");

const char version_text[] = gettext_noop("\
%s (%s) %s\n\
Copyright (c) %s %s (%s)\n\
License %s: <%s>\n\
\n\
Written by %s (%s).\n\
Github: <%s>\n");

const char win_text[] = gettext_noop("You win!");
const char continue_text[] = gettext_noop("Continue? [y/N]");

enum {
        buffer_size = 1024,

        key_escape = 27,

        car_pair = 1,
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

void initgettext()
{
        FILE *f;
        char filepath[buffer_size];
        char localepath[buffer_size];
        setlocale(LC_CTYPE, "");
        setlocale(LC_MESSAGES, "");
        strncpy(filepath, getenv("HOME"), buffer_size);
        strncat(filepath, LOCAL_LOCALEDIR"/ru/LC_MESSAGES/"PROGRAM_NAME".mo",                   buffer_size-1);
        strncpy(localepath, getenv("HOME"), buffer_size);
        strncat(localepath, LOCAL_LOCALEDIR, buffer_size-1);
        f = fopen(filepath, "r");
        if (f) {
                bindtextdomain(PROGRAM_NAME, localepath);
                fclose(f);
        } else {
                bindtextdomain(PROGRAM_NAME, LOCALEDIR);
        }
        textdomain(PROGRAM_NAME);
}

void show_help()
{
        printf(_(help_text), PROGRAM_NAME, PACKAGE_NAME, PACKAGE_PAGE);
}

void show_version()
{
        printf(_(version_text),
               PROGRAM_NAME, PACKAGE_NAME, VERSION,
               COPYRIGHT_YEAR, AUTHOR, AUTHOR_NICKNAME,
               LICENSE, LICENSE_PAGE,
               AUTHOR, AUTHOR_NICKNAME,
               AUTHOR_PAGE);
}

int handle_opt(char **argv)
{
        argv++;
        for (; *argv; argv++) {
                if (0 == strcmp(*argv, HELP_SHORT_OPT) ||
                    0 == strcmp(*argv, HELP_LONG_OPT)) {
                        show_help();
                        return 0;
                }
                if (0 == strcmp(*argv, VERSION_SHORT_OPT) ||
                    0 == strcmp(*argv, VERSION_LONG_OPT)) {
                        show_version();
                        return 0;
                } 
        }
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

void initcolors()
{
        init_pair(car_pair, car_font_color, car_bg_color);
        init_pair(barrier_pair, barrier_font_color, barrier_bg_color);
}

void initmap(struct map *m)
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

void initcar(struct car *c, int pos, int score, struct map m)
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

void initbarrier(struct barrier *b, int n, int save, struct map m)
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

void initgame(struct map *m, struct car *c, struct barrier *b, int n)
{
        int i;
        srand(time(NULL));
        timeout(max_speed);
        initmap(m);
        initcar(c, start_pos, 0, *m);
        for (i = 0; i < n; i++)
                initbarrier(&b[i], i, 0, *m);
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
        mvprintw(y, x, N_("%d"), score);
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
                        initbarrier(&b[i], i, 0, m);
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
        initmap(m);
        initcar(c, c->pos, c->score, *m);
        for (i = 0; i < n; i++)
                initbarrier(&b[i], i, 0, *m);
        check_barrier(b, n);
        draw_screen(*m, *c, b, n);
}

void playgame(struct map *m, struct car *c, struct barrier *b, int n)
{
        int res, key;
        while ((key = getch()) != key_escape) {
                show_score(c->score, 0, 0);
                switch (key) {
                case KEY_LEFT:
                case 'A':
                case 'a':
                        move_car(c, -1);
                        break;
                case KEY_RIGHT:
                case 'D':
                case 'd':
                        move_car(c, 1);
                        break;
                case KEY_RESIZE:
                        handle_resize(m, c, b, n);
                        break;
                }
                c->score++;
                show_road(*m, c->score);
                res = check_collision(*c, b, n);
                if (res)
                        break;
                if (c->score >= max_score)
                        break;
                move_barrier(b, n, *m);
                speedup(c->score);
        }
}

void endgame(int score)
{
        int x, y;
        sleep(1);
        getmaxyx(stdscr, y, x);
        clear();
        if (score < max_score)
                show_score(score, y/2, (x-3)/2);
        else
                mvprintw(y/2, (x-strlen(_(win_text)))/2, _(win_text));
        refresh();
}

int ask_continue()
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

int main(int argc, char **argv)
{
        struct map m;
        struct car c;
        struct barrier b[barrier_count];
        initgettext();
        if (!handle_opt(argv))
                return 0;
        initcurses();
        initcolors();
        do {
                initgame(&m, &c, b, barrier_count);
                draw_screen(m, c, b, barrier_count);
                playgame(&m, &c, b, barrier_count);
                endgame(c.score);
        } while (ask_continue());
        endwin();
        return 0;
}
