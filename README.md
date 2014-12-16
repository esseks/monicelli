Monicelli
=========

Monicelli is an esoterical programming language based on the so-called
"supercazzole" from the movie Amici Miei, a masterpiece of the Italian
comedy.

There is no way to translate a "supercazzola" to English, so if you don't speak
Italian, I'm afraid you won't understand. I'm really sorry for you :)

Compilation
===========

You will need `bison` version >= 3.0 (Bison 2.5 requires manual intervention),
`flex` >= 2.5 and any C++11 compiler. The build scripts are generated using
CMake. A typical Makefile-based build workflow would be:

    mkdir build/
    cd build/
    cmake ..
    make

During the Makefile generation, the build script will test the compiler for all
the required features.

If your tools are installed in non-standard locations
(e.g. Bison Brew on Mac OS X), you can alter the search path with:

    PATH=/path/to/bison cmake ..

If you can't really upgrade to Bison 3.0, a patch for Bison 2.5 
is provided in `cmake/bison2.patch`. You will have to manually apply it with:

    patch -p 1 < cmake/bison2.patch

However note that compilation with Bison 2.5 is not supported and the patch might be
removed in the future.

Usage
=====

`mcc` is a source to source compiler, which reads Monicelli and outputs a
subset of C++. For those of you who want to get to the code ASAP, the `examples/`
folder contains a set of programs covering most of the features of the language.

A good way to learn on the field is comparing the resulting C++ with the
input. Well, mostly with the beautified version of the input, `*.beauty.mc`.

The compiler reads from standard input and print result to standard output.

    $ ./mcc < examples/primes.mc > primes.cpp
    $ c++ primes.cpp -o primes
    $ ./primes

Language overview
=================

The original specification can be found in `Specification.txt`, and was
initially conceived by my colleagues and dear friends Alessandro Barenghi,
Michele Tartara and Nicola Vitucci, to whom goes my gratitude.

Unfortunately, their proposal was meant to be a joke and is not complete.
This project is an ongoing effort to produce a rigorous specification for the
language and implement a compiler, which implies filling gaps and ambiguities
with sensible choices.

Statements have no terminator, i.e. no semicolon `;` or the like. A single
statement can be split across multiple lines and multiple statements can be
grouped on the same line. However, keywords consisting of multiple space-separed
words **cannot** be split on multiple lines.

A comma might be inserted after each statement, if it fits the sentence ;)

Accented letters can be replaced by the non-accented letter followed by a
backtick `` ` ``, although the use of the correct Italian spelling is strongly
encouraged for maximizing the antani effect.

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

maps to `antani << 2`.

It goes without saying, other expression can be used instead of numbers.
Also, the usual precedence rules apply.

**Braces are not implemented**.

Variables
---------

A variable name can contain numbers, upper and lower case character and must
not start with a number (the usual rules, that's it).

A variable might be prefixed with an article to fit a sentence. The
compiler does not check concordance with the following name, but accepts any
article of the Italian language: `il`, `lo`, `la`, `i`, `gli`, `le`, `un`, `una`
`dei`, `delle`, `l'`, `un'`. For instance, `cappello` and `il cappello` refer 
to the same variable.

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

For example:

    voglio antani, Necchi come se fosse 10
    stuzzica
        antani come fosse antani meno 1
    e brematura anche, se antani maggiore di 0

maps to:

    int antani = 10;
    do {
        antani = antani - 1;
    } while (antani > 0);

`brematura` might be replaced by its alternate form `prematura`

Branch
------

The branch construct encompasses both the features of an `if` and a `switch`.
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
        o magari intuizione diviso 2:
            genio come se fosse genio più 1
        o magari maggiore di mobiletto per due:
            genio come se fosse genio per 2
        o tarapia tapioco:
            genio come se fosse 2
    e velocità di esecuzione

maps to:

    if (genio == intuizione) { 
        genio = genio - 1;
    } else if (genio == (intuizione / 2)) {
        genio = genio + 1;
    } else if (genio > (mobiletto * 2)) {
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

The `o tarapia tapioco` block can be omitted:

    che cos'è il genio?
        maggiore di mobiletto:
            genio come se fosse 2
    e velocità di esecuzione

Finally, here is the equivalent of a `switch () {}`:

    che cos'è il genio?
        1:
            genio come se fosse 2
        o magari 2:
            genio come se fosse 7
        o tarapia tapioco:
            genio come se fosse 9
    e velocità di esecuzione

where the `o tarapia tapioco` part is like the `default` block.

Functions
---------

**Note**: the alternate spelling `supercazzora` might be used in place
 of `supercazzola` wherever the latter appears.

###Declaration

A function is declared with the `blinda la supercazzola` statement:

    blinda la supercazzola [<type>] <name> [con <param> <type>[, <param> <type>...]] o scherziamo?
        <statements>

Where `<type>` can be omitted for a void function. For instance:

    blinda la supercazzola Necchi antanizzata con alfio Mascetti o scherziamo?
        vaffanzum alfio meno 2!

is a function of type `Necchi`, taking one argument of type `Mascetti`.
Multiple arguments must be comma-separed, like in:

    blinda la supercazzola Necchi antanizzata con alfio Mascetti, barilotto Necchi o scherziamo?
        vaffanzum alfio meno 2!

which is a function of type `Necchi`, taking two arguments of type `Mascetti`
and `Necchi`. It maps to:

    int antanizzata(char alfio, int barilotto) {
        return alfio - 2;
    }

Finally, this:

    blinda la supercazzola antanizzata o scherziamo?
        vaffanzum!

is a `void` function taking no arguments and becomes:

    void antanizzata() {
        return;
    }

Functions cannot be nested and can be declared before or after the main in any
order. `mcc` will not check that a return statement is always reachable inside
 a non-void function. Failing to return a value leads to undefined behaviour.

###Invocation

A function is called with the `brematurata la supercazzola` statement:

    brematurata la supercazzola <name> [con <expression>[, <expression>...] o scherziamo?

Functions might be called inside expressions. For instance, this:

    antani come se fosse brematurata la supercazzola alfio con barilotto diviso 3 o scherziamo? per 2

maps to:

    antani = alfio(barilotto / 3) * 2;

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
instance, in:

    antani come se fosse 4 bituma, scusi, noi siamo in quattro

`, scusi, noi siamo in quattro` is ignored.

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

In addition to that, meta comments are printed to `stderr` during compilation.

Reserved words and phrases
------------------------

The following phrases are currently reserved with no assigned usage. They cannot be used as variable identifiers, even if they do not serve any other purpose in the current language revision.

* `conte`
* `scusi noi siamo in`
