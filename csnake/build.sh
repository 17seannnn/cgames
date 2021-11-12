#!/bin/sh

CC=gcc
NAME=csnake
BINDIR=.
#BINDIR=/usr/local/bin
LOCALEDIR=.
#LOCALEDIR=/usr/share/locale/

case $1 in
        install)
                cd src/;
                $CC -ansi -pedantic -Wall -Og -g -lcursesw main.c -o ../$NAME
                cd ..;
                mv -f $NAME $BINDIR
                mkdir -p $LOCALEDIR/ru/LC_MESSAGES;
                msgfmt po/ru.po -o $LOCALEDIR/ru/LC_MESSAGES/$NAME.mo;;
        pot) xgettext -k="_" -f "po/POTFILES.in" -D "src" -o po/$NAME.pot;;
        mo)
                mkdir -p $LOCALEDIR/ru/LC_MESSAGES;
                msgfmt po/ru.po -o $LOCALEDIR/ru/LC_MESSAGES/$NAME.mo;;
        * | --help)
                echo "\
Usage:
build.sh [COMMAND/--OPTION]

Options:
        --help     this help

Commands:
        install    install this program
        pot        generate .pot file in po/
        mo         compile and move .mo files from po/ to locale dir";;
esac
