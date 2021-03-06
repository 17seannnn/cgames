#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libintl.h>
#include <unistd.h>
#include <time.h>
#include <curses.h>

#include "mainmenu.h"
#include "cgames.h"

#define _(STR) gettext(STR)
#define gettext_noop(STR) STR
#define N_(STR) STR

static int max_pos;

static int min_speed;
static int max_speed;

static const char  pn[]             = "car reaction";
static const char  fn[]             = ".cr";
static const char  mt[][mm_bufsize] = {
                                gettext_noop("Play game"),
                                gettext_noop("Settings"),
                                gettext_noop("Exit")
};
static const char  st[][mm_bufsize] = {
                                gettext_noop("Barrier count"),
/* Use max speed text on min speed variable to make more understandable */
                                gettext_noop("Max speed"),
                                gettext_noop("Min speed")
};
static const char  sr[][mm_bufsize] = {
                                "i", "3", "1", "1", "20",
                                "i", "50", "5", "1", "500",
                                "i", "75", "5", "1", "500",
};
static       void *sp[] = {
                                &max_pos, &min_speed, &max_speed
};
static const int   mc = 3, sc = 3;
static const int   mm_colors[mm_colors_count] = {
                                 COLOR_WHITE, COLOR_BLACK, A_STANDOUT,
                                 COLOR_WHITE, COLOR_BLACK, 0,
                                 COLOR_RED, COLOR_BLACK, A_BOLD,
                                 COLOR_RED, COLOR_BLACK, 0
};

static const char win_text[] = gettext_noop("You win!");
static const char continue_text[] = gettext_noop("Continue? [y/N]");

enum {
        buffer_size = 1024,

        key_escape = 27,

        car_pair = 1,
        barrier_pair,

        car_symbol         = 'I',
        barrier_symbol     = '=',

        max_score          = 32767,

        start_pos          = 2,
        min_pos            = 1,

        road_height        = 16,
        road_width         = 11,

        car_font_color     = COLOR_BLUE,
        car_bg_color       = COLOR_BLACK,
        barrier_font_color = COLOR_RED,
        barrier_bg_color   = COLOR_BLACK
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

static int rrand(int max)
{
        return (int)((double)max*rand()/(RAND_MAX+1.0));
}

static void initcolors()
{
        init_pair(car_pair, car_font_color, car_bg_color);
        init_pair(barrier_pair, barrier_font_color, barrier_bg_color);
}

static void initcurses()
{
        if (has_colors())
                start_color();
        initcolors();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, 1);
}

static void initmap(struct map *m)
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

static void initcar(struct car *c, int pos, int score, struct map m)
{
        c->cur_x = m.min_x + road_width * pos + pos-1 - road_width/2;
        c->cur_y = m.max_y - m.h/5;
        c->pos = pos;
        c->score = score;
}

static void check_barrier(struct barrier *b, int n)
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

static void initbarrier(struct barrier *b, int n, int save, struct map m)
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

static void initgame(struct map *m, struct car *c, struct barrier *b, int n)
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

static void show_map(struct map m)
{
        int y;
        for (y = m.min_y; y <= m.max_y; y++) {
                mvaddch(y, m.min_x, '|');
                mvaddch(y, m.max_x, '|');
        }
        refresh();
}

static void show_score(int score, int y, int x)
{
        mvprintw(y, x, N_("%d"), score);
}

static void show_road(struct map m, int score)
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

static void show_car(struct car c)
{
        mvaddch(c.cur_y, c.cur_x, car_symbol | COLOR_PAIR(car_pair));
        refresh();
}

static void hide_car(struct car c)
{
        mvaddch(c.cur_y, c.cur_x, ' ');
        refresh();
}

static void move_car(struct car *c, int dpos)
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

static void show_barrier(struct barrier *b, int n)
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

static void hide_barrier(struct barrier *b, int n)
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

static void move_barrier(struct barrier *b, int n, struct map m)
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

static int check_collision(struct car c, struct barrier *b, int n)
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

static void speedup(int score)
{
        int speed = max_speed - 5 * (score / (max_score/100));
        if (speed < min_speed)
                speed = min_speed;
        refresh();
        timeout(speed);
}

static void draw_screen(struct map m, struct car c, struct barrier *b, int n)
{
        clear();
        show_map(m);
        show_road(m, c.score);
        show_car(c);
        show_barrier(b, n);
}

static void handle_resize(struct map *m, struct car *c, struct barrier *b,
                                                        int n)
{
        int i;
        initmap(m);
        initcar(c, c->pos, c->score, *m);
        for (i = 0; i < n; i++)
                initbarrier(&b[i], i, 0, *m);
        check_barrier(b, n);
        draw_screen(*m, *c, b, n);
}

static void playgame(struct map *m, struct car *c, struct barrier *b, int n)
{
        int res, key;
        while ((key = getch()) != key_escape) {
                show_score(c->score, 0, 0);
                switch (key) {
                case KEY_LEFT:
                case 'H':
                case 'h':
                case 'A':
                case 'a':
                        move_car(c, -1);
                        break;
                case KEY_RIGHT:
                case 'L':
                case 'l':
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

static void endgame(int score)
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

void cr()
{
        int res;
        struct map m;
        struct car c;
        struct barrier *b = malloc(sizeof(*b) * max_pos);
        initmm(pn, fn, mt, st, sr, sp, mc, sc, mm_colors);
        do {
                res = mainmenu();
                if (res == exit_choise)
                        break;
                initcurses();
                initgame(&m, &c, b, max_pos);
                draw_screen(m, c, b, max_pos);
                playgame(&m, &c, b, max_pos);
                endgame(c.score);
        } while (ask_continue());
        free(b);
}
