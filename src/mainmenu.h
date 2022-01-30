/* mainmenu 1.1 */

#ifndef MAINMENU_H
#define MAINMENU_H

enum {
        mm_bufsize      = 256,
        mm_colors_count = 12,
        exit_choise     = 255
};

int mainmenu();
void initmm(const char *pn, const char *fn,
            const char (*mt)[mm_bufsize], const char (*st)[mm_bufsize],
            const char (*sr)[mm_bufsize], void *sp[],
            const int mc, const int sc, const int mm_colors[mm_colors_count]);

#endif
