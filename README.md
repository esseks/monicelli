# Monicelli 2.0 "Cofandina"

This all-new release mainly brings several improvements to the code that make
it easier to hack and build new features. In addition to that:

* `mcc` now produces an executable by default, no need to use (or install)
  `lcc`, as it was previously the case. `mcc cofandina.mc -o cofandina` and
  that's it! This feature currently requires a POSIX system (like Linux or 
  Mac OS X) with a C compiler installed (anything reasonably recent will do).

* `mcc` does not depend on Boost anymore.

* `mcc` has a new hand-written parser that should provide better error
  messages. Now it's easier to stuzzicate your prematurated supercazzole.
  Error messages are in plain English and not very antani. Apologies for that.

* `mcc` now generates code that directly calls the C standard library. This
  allows you to seamlessly link Monicelli object files with C/C++ code, without
  any extra dependency on a Monicelli standard library.

* The code generator in `mcc` has been ported to LLVM 6.0 and will continue 
  to be updated with new releases.

* Modules are gone. This was a rather obscure feature that allowed to expose
  functions implemented in C/C++ to Monicelli code using a YAML-based language.
  Instead, it's now possible to declare a function with an empty body to signal
  that it will be implemented in another file, be it in Monicelli or C/C++.
  See the updated Turtle example.

* The C++ transpiler is gone. It might come back again, though.

# What's Monicelli anyway?

Monicelli is an esoterical programming language based on the so-called
"supercazzole" from the movie Amici Miei, a masterpiece of the Italian
comedy.

Over the past few years I have tried to render the idea of "supercazzola" to
non-Italian speakers, with little success. The closest I got was by describing
it as "comically deceptive gibberish", which sadly does not capture the true
essence of what a "supercazzola" (spelled "supercazzora" according to some) is.
I'm still open to suggestions on how to better present Monicelli (the language)
to the international public.

# Compilation

A part of the Monicelli compiler (the lexer) is generated using `ragel`, which
you will need to have installed. If this is not the case, the configuration
script will warn you. Monicelli is developed with version 6.8, but any
sufficiently recent release should do just fine.

You will also need to have LLVM development libraries installed, version 7, 8 or 9.
Other versions might or might not work. CMake looks for version 7 by default, you
can override this by setting the `MONICELLI_LLVM_VERSION` variable:

    $ cmake -DMONICELLI_LLVM_VERSION=9

Finally, you will need CMake, version 3.7 or higher.

A typical Makefile-based build workflow would be:

    $ cd monicelli/
    $ mkdir build/
    $ cd build/
    $ cmake .. -DCMAKE_INSTALL_PREFIX="$HOME/mcc"
    $ make all install

If your tools are installed in non-standard locations
(e.g. Homebrew on Mac OS X), you can alter the search path with:

    $ PATH=/path/to/ragel cmake ..

`mcc` statically links LLVM, once compiled it will only depend on the C++
runtime and on `libz`.

## Note for non-POSIX platforms (like Windows)

The external linker is called using fork+exec for simplicity. This means that
this part of the workflow will **not** work on non-POSIX systems, such as
Windows. There, you will need to disable this feature at build time. You will
only get object files (.o) that you will have to link, including a C runtime
library, by yourself.

You can disable the invocation of an external linker and make `mcc` compilable
on Windows during CMake configuration by forcing the appropriate flag to OFF:

    $ cmake .. -DMONICELLI_LINKER=OFF

## Tested platforms

The reference OS for building and testing Monicelli is Ubuntu 16.04 LTS. If the
build is broken there, then it's a bug. Unfortunately I don't have many other
platforms at hand to test, but it _should_ compile on many more POSIX systems, 
including Mac OS X. If you managed to compile Monicelli on your favourite
platform and you needed a patch, it would be great if you could send a PR.

# Usage

Monicelli build an executable by default on POSIX systems
(such as Linux, Mac OS X). Linking requires an external C compiler, anything
decently modern and standard-conformant should do.

A typical invocation is very similar to what you would expect from your C
compiler:

    $ mcc example.mc -o example
    $ ./example

Please be aware that the Monicelli compiler depends on the availability of a C
compiler and stdlib, although this dependency should be available on virtually
all platforms where you might think to run `mcc`.

# Language overview

The original specification can be found in `Specification.txt`, and was
initially conceived by my colleagues and dear friends Alessandro Barenghi,
Michele Tartara and Nicola Vitucci, to whom goes my gratitude.

Their proposal was meant to be an elaborate joke and is not complete.
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

## Getting started real quick

For those of you who want to get to the code ASAP, the `examples/`
folder contains a set of programs covering most of the features of the language.


## Main

The entry point of the program (the "main") is identified by the phrase:

    Lei ha clacsonato

which marks the beginning of the _supercazzola_ (i.e. of the program).

A value can be returned by using the the following statement:

    vaffanzum <expression>!

optionally, no value might be returned with:

    vaffanzum!

## Expressions

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

So `2 più 4` means `2 + 4`. `maggiore o uguale` is admitted as alternate
form of the >= operator, same for `minore o uguale`.

When evaluating binary expressions whose operands have different types,
the type of the result will be the less restrictive between the two.
This ensures that no loss takes place when evaluating an expression.

## Binary shift

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

**There is no syntax for braces in Monicelli**.

## Variables

A variable name can contain numbers, upper and lower case character and must
not start with a number (the usual rules, that's it).

A variable might be prefixed with an article to fit a sentence. The
compiler does not check concordance with the following name, but accepts any
article of the Italian language: `il`, `lo`, `la`, `i`, `gli`, `le`, `un`, `una`
`dei`, `delle`, `l'`, `un'`. For instance, `cappello` and `il cappello` refer 
to the same variable.

Consequently, the articles above cannot be used as variable names.

## Assignment

A value can be assigned to a variable with the following statement:

    <varname> come fosse <expression>

the alternate spelling `come se fosse` can be used as well.

The `<expression>` initializer is casted to the declared type of the variable,
even if the cast will cause some loss. This feature can be (ab)used to introduce
C-style casts too.

## Declaration

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

## Input/Output

Variables and expressions can be printed with the statement:

    <expression> a posterdati

Conversely, a variable might be read from input using:

    mi porga <varname>

## Loop

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

## Branch

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

## Functions

**Note**: the alternate spelling `supercazzora` might be used in place
 of `supercazzola` wherever the latter appears.

## Declaration

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

A function might be declared with no body, in which case it's treated as a
prototype. A prototype makes the function signature known to the compiler, and
it signals that the function is implemented in another file.

## Invocation

A function is called with the `brematurata la supercazzola` statement:

    brematurata la supercazzola <name> [con <expression>[, <expression>...] o scherziamo?

Functions might be called inside expressions. For instance, this:

    antani come se fosse brematurata la supercazzola alfio con barilotto diviso 3 o scherziamo? per 2

maps to:

    antani = alfio(barilotto / 3) * 2;

## Exceptions

The program might be aborted immediately with the statement:

    avvertite don ulrico

there are no arguments.

## Assertions

An assertion block will evaluate its expression and trigger an error message
if it is found to be 0 (logical false). An assertion is stated as:

    ho visto <expression>!

## Comments

Any character after `bituma` is ignored until a line break is encountered. For
instance, in:

    antani come se fosse 4 bituma, scusi, noi siamo in quattro

`, scusi, noi siamo in quattro` is ignored.

Comments are useful to fill the "supercazzola" and make it more readable, since
any word (including reserved words) can be inserted into it.

## Meta comments

In addition to line comments, there are meta comments. A meta comment starts
with an hash sign `#` and continues until a line break is encountered, as an 
ordinary comment.

They have a different graphical symbol, which can be immediately spotted inside
a long "supercazzola". Also, ordinary comments can and should be used in an 
improper way to fill the sentence, meta comments provide a mechanism for
distiguishing "real" comments.

## Reserved words and phrases

The following phrases are currently reserved with no assigned usage. They cannot
be used as variable identifiers, even if they do not serve any other purpose in
the current language revision.

* `conte`
* `scusi noi siamo in`
* `con rinforzino`
