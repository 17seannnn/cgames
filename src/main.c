#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <locale.h>
#include <libintl.h>

#include "mainmenu.h"
#include "cgames.h"

#define TEXTDOMAIN "cgames"
#define LOCALEDIR "/usr/share/locale"
#define LOCAL_LOCALEDIR ".local/share/locale"
#define LOCALEFILE_PATH "ru/LC_MESSAGES"

#define _(STR) gettext(STR)
#define N_(STR) STR
#define gettext_noop(STR) STR

enum {
        bufsize = 1024,

        game_cr = 0,
        game_csnake
};

static const char  pn[]             = "cgames";
static const char  fn[]             = ".cgames";
static const char  mt[][mm_bufsize] = {
                                 N_("car reaction"),
                                 N_("csnake"),
                                 gettext_noop("Exit")
};
static const int   mc = 3, sc = 0;
static const int   mm_colors[mm_colors_count] = {
                                 COLOR_WHITE, COLOR_BLACK, A_STANDOUT,
                                 COLOR_WHITE, COLOR_BLACK, 0,
                                 COLOR_RED, COLOR_BLACK, A_BOLD,
                                 COLOR_RED, COLOR_BLACK, 0
};

static const char program_name[]      = N_("launcher");
static const char program_name_long[] = N_("cgames");
static const char program_page[]      = N_("https://github.com/17seannnn/cgames");
static const char version[]           = N_("4");

static const char copyright_year[] = N_("2021");
static const char license[]        = N_("GPLv3");
static const char license_page[] = N_("https://www.gnu.org/licenses/gpl-3.0.en.html");

static const char author[]          = gettext_noop("Sergey Nikonov");
static const char author_nickname[] = N_("17seannnn");
static const char author_page[]     = N_("https://github.com/17seannnn");

static const char help_opt_short[]    = N_("-h");
static const char help_opt_long[]     = N_("--help");
static const char version_opt_short[] = N_("-V");
static const char version_opt_long[]  = N_("--version");

static const char help_text[] = gettext_noop("\
Usage: %s [-OPT/--OPT]\n\
\n\
Options\n\
\n\
%s, %s      show help\n\
%s, %s   show version\n\
\n\
Report bugs to & %s home page: <%s>\n");

static const char version_text[] = gettext_noop("\
%s (%s) %s\n\
Copyright (C) %s %s (%s)\n\
License %s: <%s>\n\
\n\
Written by %s (%s).\n\
Github: <%s>\n");

static void initgettext()
{
        char localdir[bufsize], localfile[bufsize];
        FILE *f;
        setlocale(LC_CTYPE, "");
        setlocale(LC_MESSAGES, "");

        strncpy(localdir, getenv("HOME"), bufsize);
        strncat(localdir, "/", bufsize-1);
        strncat(localdir, LOCAL_LOCALEDIR, bufsize-1);

        strncpy(localfile, localdir, bufsize);
        strncat(localfile, "/", bufsize-1);
        strncat(localfile, LOCALEFILE_PATH, bufsize-1);
        strncat(localfile, "/", bufsize-1);
        strncat(localfile, program_name_long, bufsize-1);
        strncat(localfile, ".mo", bufsize-1);

        f = fopen(localfile, "r");
        if (f) {
                fclose(f);
                bindtextdomain(TEXTDOMAIN, localdir);
        } else {
                bindtextdomain(TEXTDOMAIN, LOCALEDIR);
        }
        textdomain(TEXTDOMAIN);
}

static void show_help()
{
        printf(_(help_text), program_name,
                             help_opt_short, help_opt_long,
                             version_opt_short, version_opt_long,
                             program_name_long, program_page);
}

static void show_version()
{
        printf(_(version_text), program_name, program_name_long, version,
                                copyright_year, _(author), author_nickname,
                                license, license_page,
                                _(author), author_nickname,
                                author_page);
}

static int handle_opt(const char **argv)
{
        argv++;
        for ( ; *argv; argv++) {
                if (0 == strcmp(*argv, help_opt_short) ||
                    0 == strcmp(*argv, help_opt_long)) {
                        show_help();
                        return 0;
                }
                else if (0 == strcmp(*argv, version_opt_short) ||
                         0 == strcmp(*argv, version_opt_long)) {
                        show_version();
                        return 0;
                }
        }
        return 1;
}

static void mainmenu_handle(int res)
{
        switch (res) {
        case game_cr:
                cr();
                break;
        case game_csnake:
                csnake();
                break;
        default:
                break;
        }
}

int main(int argc, char **argv)
{
        int res;
        initgettext();
        res = handle_opt((const char **)argv);
        if (!res)
                return 0;
        initmm(pn, fn, mt, NULL, NULL, NULL, mc, sc, mm_colors);
        for (;;) {
                initscr();
                res = mainmenu();
                endwin();
                if (res == exit_choise)
                        break;
                mainmenu_handle(res);
        }
        return 0;
}
