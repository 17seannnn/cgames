#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <locale.h>
#include <libintl.h>

#define TEXTDOMAIN "cgames"
#define LOCALEDIR "/usr/share/locale"
#define LOCAL_LOCALEDIR ".local/share/locale"
#define LOCALEFILE_PATH "ru/LC_MESSAGES"

#define _(STR) gettext(STR)
#define N_(STR) STR
#define gettext_noop(STR) STR

#include "mainmenu.h"

enum {
        bufsize = 1024,

        cr = 0,
        csnake
};

const char  pn[]             = "cgames";
const char  fn[]             = ".cgames";
const char  mt[][mm_bufsize] = { 
                                 N_("car reaction"),
                                 N_("csnake"),
                                 gettext_noop("Exit")
};
const char  st[1][mm_bufsize];
const char  sr[1][mm_bufsize];
      void *sp[1];
const int   mc = 3, sc = 0;
const int   mm_colors[mm_colors_count] = {
                                 COLOR_WHITE, COLOR_BLACK, A_STANDOUT,
                                 COLOR_WHITE, COLOR_BLACK, 0,
                                 COLOR_RED, COLOR_BLACK, A_BOLD,
                                 COLOR_RED, COLOR_BLACK, 0
};
const int settings_menu = 0;

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
        f = fopen(localfile, "r");
        if (f) {
                fclose(f);
                bindtextdomain(TEXTDOMAIN, localdir);
        } else {
                bindtextdomain(TEXTDOMAIN, LOCALEDIR);
        }
        textdomain(TEXTDOMAIN);
}

static void mainmenu_handle(int res)
{
        switch (res) {
        case cr:
                system("cr");
                break;
        case csnake:
                system("csnake");
                break;
        default:
                break;
        }
}

int main()
{
        int res;
        initgettext();
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
