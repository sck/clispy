clispy - A scheme-like language in 800 lines (27K) of C code
============================================================

A port of Peter Norvig's lis.py (http://norvig.com/lispy.html) to C.
It's 800 uncommented, non-blank lines of C code (27K). 

MOTIVATION
==========

I've always wanted to use the memory model I used in localmemcache for
implementing more than just a hashtable, so when I saw Peter Norvig's
simple parser for a scheme-like language (just 93 LoC in Python) I
felt it would be great to port it to C.

IDEA
====

clispy does its own memory management which is based on a virtual address
range of anonymously mapped memory.  This virtual address space (60G) is
made up of segments of 64KB (so a single variable cannot become larger
than that).  The idea is that on a 64bit system one has actually enough
virtual addresses that one can work with this simplified model quite
comfortably.   What it does is that it radically simplifies  memory
allocation and garbage collection (together about 80 LoC).  

On top of the memory management data types are implemented: hash, array,
string, implementing the methods like cl_string_replace etc that are
needed by the scheme parser.

The original 93 lines of python code are 168 lines (6K) in C:

  https://github.com/sck/clispy/blob/master/scheme-parser.c


USING
=====

clispy % ./compile.sh && clispy -v ./tests/fact.scm

    #! /bin/sh -v
    gcc -o clispy clispy.c
    #! ./clispy
    (define area (lambda (r) (* 3.141592653 (* r r))))
    (begin (display (area 3.0)) (newline))
    28.274334
    (define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))
    (begin (display (fact 15)) (newline))
    2004310016
    (define first car)
    (define rest cdr)
    (define count (lambda (item L) (if L (+ (equal? item (first L)) (count
    item (rest L))) 0)))
    (begin (display (count 0 (list 0 1 2 3 0 0))) (newline))
    3
    (begin (display (count (quote the) (quote (the more the merrier the
    bigger the better)))) (newline))
    4
    (define make-account (lambda (balance) (lambda (amt)  (begin (display
    balance) (newline) (set!  balance (+ balance amt)) balance))))
    (define a1 (make-account 100.00))
    (begin (display (a1 -20.00)) (newline))
    100.000000
    80.000000


LIMITATIONS
===========

* Numbers are represented by either floats or ints.
* The size of a variable is limited by CL_STATIC_ALLOC_SIZE (default: 64K) 
* The number of available variables is limited by 
   (CL_MEM_SIZE / CL_STATIC_ALLOC_SIZE) (default mem size: 60G)
* There are probably plenty of bugs in the interpreter since I haven't
worked much yet with lisp-based languages, and also I have been more
interested in designing the architecture of clispy rather than in using it.
