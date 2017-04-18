Monicelli
=========

Monicelli is an esoterical programming language based on the so-called
"supercazzole" from the movie Amici Miei, a masterpiece of the Italian
comedy.

There is no way to translate a "supercazzola" to English, so if you don't speak
Italian, I'm afraid you won't understand. I'm really sorry for you :)

Compilation
===========

You will need `bison` version >= 3.0 (Bison 2.5 works but requires manual intervention),
`flex` >= 2.5, `LLVM` >= 3.5, `Boost` >= 1.48, `YAML-cpp` >= 0.5 and any C++11 compiler.
The build scripts are generated using CMake, version >= 2.8.

A typical Makefile-based build workflow would be:

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

### Building with LLVM on Debian/Ubuntu
Debian Testing and Ubuntu >= 14.04 distribute a LLVM 3.5 development package
**which is broken** (see [1](https://bugs.launchpad.net/ubuntu/+source/llvm/+bug/1365432)
and [2](https://bugs.launchpad.net/ubuntu/+source/llvm/+bug/1387011)).

Luckly, LLVM.org directly provides an APT repo which works fine.
http://llvm.org/apt/ have all the relevant info for installing the repo.
After that, the package we need is `llvm-3.5-dev`.

**This is only necessary for compilation, Debian/Ubuntu LLVM runtime libs
and utilities work just fine.**

Usage
=====

### LLVM frontend
Monicelli emits LLVM bitcode in its default configuration.
A typical compilation workflow would be:

    $ ./mcc example.mc
    $ llc example.bc
    $ cc example.s libmcrt.a -o example

In particular, note that the Monicelli runtime library must be compiled in or linked to use
all of the I/O functions. Also note the use of the `llc` utility, which is
provided by LLVM, to produce native assembler from LLVM bitcode.

Please be aware that the Monicelli standard library depends on the C stdlib,
although this dependency is available on virtually any platform you might
dream of compiling Monicelli on.

As such, `llvm` utilities are needed for compiling. Only the "low level"
utilities (`opt` and `llc`) are needed, not the whole Clang/Clang++ suite.
Usually, the relevant package goes under the name `llvm`.

A C compiler is used to simplify the assembling and linking step, but it could
be skipped altogether with a small effort. If you want to try ;)

`mcc` only performs minimal optimizations in order to ensure readibility when
disassembling with `llvm-dis`. However, you might want to optimize the code
using `opt` LLVM utility:

    $ opt example.bc | llc -o example.s

in place of the simple `llc` compilation step. See `opt` documentation for a 
comprehensive list of optimizations available.

### C++ transpiler
`mcc` also works as a source to source compiler, which reads Monicelli
and outputs a subset of C++. Use the option `--c++` or `-+` for that.

A good way to learn on the field is comparing the resulting C++ with the
input. Well, mostly with the beautified version of the input, `*.beauty.mc`.

The typical command line would be:

    $ ./mcc --c++ examples/primes.mc
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

### Get started!
For those of you who want to get to the code ASAP, the `examples/`
folder contains a set of programs covering most of the features of the language.


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

When evaluating binary expressions whose operands have different types,
the type of the result will be the less restrictive between the two.
This ensures that no loss takes place when evaluating an expression.

### Binary shift

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

### Assignment

A value can be assigned to a variable with the following statement:

    <varname> come fosse <expression>

the alternate spelling `come se fosse` can be used as well.

The `<expression>` initializer is casted to the declared type of the variable,
even if the cast will cause some loss. This feature can be (ab)used to introduce
C-style casts too.

### Declaration

Variables can be declared in any scope. There are 5 variable types, which are
directly mapped on C++/C99 types as follows: 

| Type name | Mapped C type | Size  |
|-----------|---------------|-------|
| Necchi    | `int`         | 64bit |
| Mascetti  | `char`        |  8bit |
| Perozzi   | `float`       | 32bit |
| Melandri  | `bool`        |     - |
| Sassaroli | `double`      | 64bit |

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

### Declaration

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

### Invocation

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

### Meta comments

In addition to line comments, there are meta comments. A meta comment starts
with an hash sign `#` and continues until a line break is encountered, as an 
ordinary comment.

They have a different graphical symbol, which can be immediately spotted inside
a long "supercazzola". Also, ordinary comments can and should be used in an 
improper way to fill the sentence, meta comments provide a mechanism for
distiguishing "real" comments.

Reserved words and phrases
------------------------

The following phrases are currently reserved with no assigned usage. They cannot be used as variable identifiers, even if they do not serve any other purpose in the current language revision.

* `conte`
* `scusi noi siamo in`
* `con rinforzino`
