#!/bin/sh

CC=gcc
NAME=cgames
if sudo -v 2> /dev/null; then
        BINDIR=/usr/bin
        LOCALEDIR=/usr/share/locale/
else
        BINDIR=~/.local/bin
        LOCALEDIR=~/.local/share/locale/
fi

case $1 in
        install)
                $CC -ansi -pedantic -Wall -Og -g -lncursesw -c src/mainmenu.c
                $CC -ansi -pedantic -Wall -Og -g -lncursesw src/mainmenu.o src/main.c -o $NAME
                msgfmt po/ru.po -o $NAME.mo
                mkdir -p $BINDIR
                mv -f $NAME $BINDIR
                mkdir -p $LOCALEDIR/ru/LC_MESSAGES
                mv $NAME.mo $LOCALEDIR/ru/LC_MESSAGES;;
        pot) xgettext -k="_" -f "po/POTFILES.in" -D "src" -o po/$NAME.pot;;
        * | --help) echo "\
Usage:
build.sh [COMMAND/--OPTION]

Commands:
        install    install this program
        pot        generate .pot file in po/
        help       show help";;
esac
