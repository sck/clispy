#! /bin/sh

perl -ne 'print if /^\s*["\-\\(#{;a-zA-Z}]/' clispy.c > a.c
gcc -o a a.c && wc -l a.c
