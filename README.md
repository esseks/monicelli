Monicelli
=========

Monicelli is an esoterical programming language based on the so-called
"supercazzole" from the movie Amici Miei, a masterpiece of the Italian
comedy.

There is no way to translate a "supercazzola" to English, so if you don't speak
Italian, I'm afraid you won't understand. I'm really sorry for you :)

Compilation
===========

You will need `bison` version > 3.0, `flex` 2.5 or above and any C++11 compiler.
A makefile is provided and will compiler the `mcc` executable. Compiling the
executable is a matter of:

    make

there are some other targets, which are of interest only for developers.

Usage
=====

`mcc` is a source to source compiler, which reads Monicelli and outputs a
subset of C++. For those of you who want to get to the code ASAP, the `examples/`
folder contains a set of programs covering most of the features of the language.

A good wat to learn on the field  is comparing the resulting C++ with the
input. Well, mostly with the beautified version of the input, `*.beauty.mc`.

The compiler reads from standard input and print result to standard output.

    $ ./mcc < examples/primes.mc > primes.cpp
    $ c++ primes.cpp -o primes
    $ ./primes

Language overview
=================

The original specification can be found in `Specification.txt`, which
unfortunately is not complete. This project is an ongoing effort to implement
it, which means filling gaps and ambiguities. This file only documents
usable features of the language.

Statements have no terminator, i.e. no semicolon `;` or the like. A single
statement can be split across multiple lines and multiple statements can be
grouped on the same line. However, keywords consisting of multiple space-separed
words **cannot** be split on multiple lines.

A comma might be inserted after each statement, if it fits the sentence ;)

Main
----

The entry point of the program (the "main") is identified by the phrase:

    Lei ha clacsonato

which marks the beginning of the _supercazzola_ (i.e. of the program).

A value can be returned by using the the following statement:

    vaffanzum <expression>!

optionally, no value might be returned with:

    vaffanzum!

Expressions
-----------
The usual operators are given, but spelled as words to best fit in sentences.
They are directly mapped on usual operators as follows:

| Form                 | Maps to |
|----------------------|---------|
| più                  | `+`     |
| meno                 | `-`     |
| per                  | `*`     |
| diviso               | `/`     |
| maggiore di          | `>`     |
| minore di            | `<`     |
| maggiore uguale a/di | `>=`    |
| minore uguale a/di   | `<=`    |

So `2 più 4` means `2 + 4`.

###Binary shift

Binary shift operators have a slighly different
syntax:

    <what> con scappellamento a <direction> per <bits>

which is equivalent to `what >> bits` or `what << bits`, depending on the 
direction, which is specified as follows:

| Phrase   | Direction |
|----------|-----------|
| destra   | right     |
| sinistra | left      |

as you might have noticed, those are simply the translation in Italian of
"left" and "right". For instance:

    antani con scappellamento a sinistra per 2

maps to `antani >> 2`.

It goes without saying, other expression can be used instead of numbers.
Also, The usual precedence rules apply.

**Braces are not implemented**.

Variables
---------

A variable name can contain numbers, upper and lower case character and must
not start with a number (the usual rules, that's it).

A variable might be prefixed with an article to fit a sentence. The
compiler does not check concordance with the following name, but accepts any
article of the Italian language: `il`, `lo`, `la`, `i`, `gli`, `le`, `un`, `una`
`dei`, `l'`, `un'`. For instance, `cappello` and `il cappello` refer to the
same variable.

Consequently, the articles above cannot be used as variable names.

###Assignment

A value can be assigned to a variable with the following statement:

    <varname> come fosse <expression>

the alternate spelling `come se fosse` can be used as well.

###Declaration

Variables can be declared in any scope. There are 5 variable types, which are
directly mapped on C++/C99 types as follows: 

| Type name | Mapped C type |
|-----------|---------------|
| Necchi    | `int`         |
| Mascetti  | `char`        |
| Perozzi   | `float`       |
| Melandri  | `bool`        |
| Sassaroli | `double`      |

A variable is declared with the following statement:

    voglio <varname>, <type>

an initialization value can be provided:

    voglio <varname>, <type> come se fosse <expression>

for instance:

    voglio antani, Necchi come se fosse 4

declares a variables called `antani` of type `Necchi` (`int`) and initializes
it to 4.

Input/Output
------------

Variables and expressions can be printed with the statement:

    <expression> a posterdati

Conversely, a variable might be read from input using:

    mi porga <varname>

Loop
----

There is only one loop construct, equivalent to a C `do {} while();`, which is
defined as follows:

    stuzzica
        <statements>
    e brematura anche, se <condition>

Branch
------

The branch construct encompasses both the features of an `if` and of a `switch`.
The best way to explain it is by comparing its various forms to the corresponding
C translation.

This is the general form:

    che cos'è <variable>?
        <condition>:
            <statements>
        o magari <condition>:
            <statements>
        o tarapia tapioco:
            <statement>
    e velocità di esecuzione

where `<condition>` might be either a value or a semi-expression, that is an
operator followed by any expression. For instance:

    che cos'è il genio?
        intuizione:
            genio come se fosse genio meno 1
        maggiore di mobiletto:
            genio come se fosse genio per 2
        o tarapia tapioco:
            genio come se fosse 2
    e velocità di esecuzione

maps to:

    if (genio == intuizione) { 
        genio = genio - 1;
    } else if (genio > mobiletto) {
        genio = genio * 2;
    } else {
        genio = 2;
    }

The statement can emulate an `if () {} else {}`:

    che cos'è il genio?
        maggiore di mobiletto:
            genio come se fosse 2
        o tarapia tapioco:
            genio come se fosse 0
    e velocità di esecuzione

Placing multiple `o <condition>:` block is similar to a chain of `else if` in C.

and a `switch () {}`:

    che cos'è il genio?
        1:
            genio come se fosse 2
        o 2:
            genio come se fosse 7
        o tarapia tapioco:
            genio come se fosse 9
    e velocità di esecuzione

where the `o tarapia tapioco` part is like the `default` block.

Exceptions
----------

The program might be aborted immediately with the statement:

    avvertite don ulrico

there are no arguments.

Assertions
----------

An assertion block will evaluate its expression and trigger an error message
if it is found to be 0 (logical false). An assertion is stated as:

    ho visto <expression>!

Comments
--------

Any character after `bituma` is ignored until a line break is encountered. For
instance:

    antani come se fossee 4 bituma lorem ipsum

Comments are useful to fill the "supercazzola" and make it more readable, since
any word (including reserved words) can be inserted into it.

###Meta comments

In addition to line comments, there are meta comments. A meta comment starts
with an hash sign `#` and continues until a line break is encountered, as an 
ordinary comment.

They have a different graphical symbol, which can be immediately spotted inside
a long "supercazzola". Also, ordinary comments can and should be used in an 
improper way to fill the sentence, meta comments provide a mechanism for
distiguishing "real" comments.

In addition to that, meta comments are printed to  `stderr` during compilation.

