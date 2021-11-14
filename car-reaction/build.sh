#!/bin/sh

CC=gcc
NAME=cr
if sudo -v 2> /dev/null; then
        BINDIR=/usr/bin
        LOCALEDIR=/usr/share/locale/
else
        BINDIR=~/.local/bin
        LOCALEDIR=~/.local/share/locale/
fi

case $1 in
        install)
                $CC -ansi -pedantic -Wall -Og -g -lncursesw src/main.c -o $NAME
                msgfmt po/ru.po -o $NAME.mo
                mv -f $NAME $BINDIR
                mkdir -p $LOCALEDIR/ru/LC_MESSAGES
                mv $NAME.mo $LOCALEDIR/ru/LC_MESSAGES/;;
        pot) xgettext -k="_" -f "po/POTFILES.in" -D "src" -o po/$NAME.pot;;
        mo)
                mkdir -p $LOCALEDIR/ru/LC_MESSAGES
                msgfmt po/ru.po -o $LOCALEDIR/ru/LC_MESSAGES/$NAME.mo;;
        * | --help) echo "\
Usage:
build.sh [COMMAND/--OPTION]

Options:
        --help     this help

Commands:
        install    install this program
        pot        generate .pot file in po/
        mo         compile and move .mo files from po/ to locale dir";;
esac
