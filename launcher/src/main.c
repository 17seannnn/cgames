#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <locale.h>
#include <libintl.h>

#define TEXTDOMAIN "mmtest"
#define LOCALEDIR "/usr/share/locale"
#define LOCAL_LOCALEDIR ".local/share/locale"

#define _(STR) gettext(STR)
#define _N(STR) STR
#define gettext_noop(STR) STR

#include "mainmenu.h"

enum {
        cr = 0,
        csnake
};

const char  pn[]             = gettext_noop("Test");
const char  fn[]             = ".cgames";
const char  mt[][mm_bufsize] = { 
                                 gettext_noop("car reaction"),
                                 gettext_noop("csnake"),
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
        setlocale(LC_CTYPE, "");
        setlocale(LC_MESSAGES, "");
        bindtextdomain(TEXTDOMAIN, LOCALEDIR);
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
