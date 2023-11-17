______________________________________________________________
# SEABASS
______________________________________________________________

![logo](logo.png)
______________________________________________________________

[Official Documentation and Introduction](https://gek169.github.io/seabass/)
[Tutorials](https://github.com/gek169/funbas)


Seabass is a public domain, self-retargeting, self-extending, 
self-modifying metaprogramming language and general-purpose 
metaprogramming tool.

What does that mean?

## Public Domain

Seabass is a public domain dedication, so that your joy
may be full. No attribution required.

## Language

Seabass is, fundamentally, a programming language. You can write
software with it. The syntax of the language is fairly unique,
however it is close cousins with Lua, BASIC, and C.

## Tool

Seabass is more than just a single language and single compiler,
it is a general-purpose software creation assistance tool. Using
Seabass, you can compile arbitrary code for arbitrary targets with
as many (or as few) layers of abstraction in-between as you desire.

## General-Purpose

Seabass aims to "fill the gap" for virtually every conceivable
software-authorship use case. It is not a domain specific
language.

## Self-retargeting

Seabass requires that the compilation unit write code to define how it
should be compiled. This code is given virtually unrestricted access to
the internals of the compiler and runs at compiletime. It is responsible
for generating "target code" (such as machine code) using all of the functions,
types, methods, and global variables in the unit.

## Self-extending

Seabass allows the definition of special `codegen` functions, methods, and
global variables which exist exclusively at compiletime. 

These functions, methods, and variables effectively extend the capabilities 
of the compiler, and should be thought of as a natural extension of the
compiler.

## Self-modifying

Seabass allows `codegen` code to manipulate the internal state of the
compiler, including the abstract syntax tree (AST, the internal
representation of a program) and token stream.

## Metaprogramming

Seabass includes a special operator (@) for invoking `codegen` functions
to "take over" parsing. They can perform arbitrary manipulations of the
program's source code (as represented in the token stream).

In practice, this allows you to not only define entirely new syntaxes,
language constructs, and abstractions, but entirely new programming languages.



# Example program

Here is a minimal complete example program, which you can find under 'tests2/toc_fib_example.cbas'

```c

#include <toc_native_user>


/*
    Metaprogramming- Defining a custom syntax.
    
    We take the existing DSL for pretty-printing
    called "pprint" and mutate it by dependency injection.
*/

@wksht prnt [
    [
        @pprint [println mutoa ARG1]
    ][
        ARG1
    ]
]


fn predecl mutoa(char* dest, uint value);
fn predecl matou(char* in)->uint;

fn inline fib(uint n)->uint:
    if(n < 2)
        return 1;
    end
    uint a=1
    uint b=1
    uint c=1;
    n = n-2
    while(n)
        c = a + b;
        a = b;
        b = c;
        n--
    end
    return c
end


fn pub main(int argc, schar** argv)->int:
    if(argc < 2)
        @prnt[
            /       "Usage: fib 13"
        ]
        sys_exit(1);
    end
    println("Welcome to the fibonacci number calculator!");
    uint qq = matou((char*)argv[1]);
    @prnt[
        /       "You asked for the fibonacci number..."
        /int    (qq)
        /       "That fibonacci number is:"
        /int    (fib(qq))
    ]
    return 0;
end

/*
    Our code generator...
*/
fn codegen codegen_main():
    cg_emitC(SEABASS_STDLIB_PREFIX);
end

/*
    EXAMPLE: Low level library code...
*/
fn matou(char* in)->uint:
    /*
        Decimal only...
    */
    uint retval = 0;
    while(
        (in[0] >= '0') && 
        (in[0] <= '9')
    )
        retval = retval * 10;
        retval = retval + (in[0]-'0');
        in++
    end
    return retval;
end


fn mutoa(char* dest, uint value):
    if(value == 0)
        dest[0] = '0';
        dest[1] = 0;
        return 
    end
    /*Determine the highest power of 10.*/
    if(1)
        uint power
        power = 1;
        while(value/power >= 10)
            power = power * 10; 
        end
        /*found the highest powerer of 10*/
        while(power)
            uint temp
            temp = value/power; /*if we had the number 137, we would have gotten
            100 as our power. We now divide by power to get the highest digit.*/
            
            dest[0] = (temp + ('0')); dest++;
            
            value = value - temp * power; /*Get rid of the highest digit.*/
            
            power = power / 10 /*Divide power by 10.*/
        end
    end

    :ending
    dest[0] = 0

    return;
end
```
Here is another example program which uses the standard library. It prints the contents of a directory:

```c
#include <toc_native_user>
//mutate pprint....
@wksht prnt[
    [
        @pprint [println itoa ARG1]
    ][
        ARG1
    ]
]


fn pub main(i32 argc, char** argv)->i32:
    if(argc < 2)
        @prnt[
            /   "Usage:"
            /   "dirlist /path/to/the/directory"
        ]
        sys_exit(1);
    end 
    u32[1] nentries;
    char**[1] listing;
    errno = 0;
    if(
        getdirlist(argv[1], listing, nentries)
    )
        @prnt[
            /   "Could not load directory:"
            /   (argv[1])
        ]
        sys_exit(1);
    end
    if(errno)
        perror("<C library> Error:");
        sys_exit(1);
    end
    
    u64 i
    for(i = 0; i < nentries[0]; i++)
        println(listing[0][i]);
    end
end

fn codegen codegen_main():
    cg_emitC(SEABASS_STDLIB_PREFIX);
end
```


# Seabass Superfeatures

1. Arbitrary compiletime execution. Seabass lets you write arbitrary code which runs at compiletime,
    called "codegen" code.

2. Compiler Reflection. Seabass lets compiletime ("codegen") code see the internal state of the compiler
    and even manipulate it. Furthermore, you can define new state variables and data structures, meaning
    you can effectively extend the compiler by writing code it understands, during compilation.

3. Parsehooks, aka "Parser Hooks".
    Seabass lets you write functions which temporarily "take over" parsing from the compiler, letting you
    define totally new syntaxes. You can manipulate the input fed to the parser as well
    as the internal representation of code which has already been parsed (The Abstract Syntax Tree,
    or "AST").
    
4. Code generators. Seabass allows you to define how your code will compile into some target code.
    This means that, unlike pretty much every other compiler, cross-compilation does not require
    an entirely new toolchain. All you need is a new code generator. You don't have to compile
    a totally new compiler just because you want to compile for the M68k- you just need
    a new code generator.
    
Because of these "superfeatures" the following are all true with Cbas:

* You can write software which is theoretically infinitely portable. If a particular computer
    platform has the features necessary to implement the program (i.e. enough memory, network
    access, a filesystem...) then the only "new" thing that needs to be written is a code
    generator for that target, and once that is written, all you have to do is #include it
    in your code!
    
* You can write totally new programming languages easily. If you want to write your own programming
    language, all you have to do is write a parsehook in Seabass which compiles your language into
    seabass (which is made much easier with the wide variety of metaprogramming tools available to
    you). This isn't just restricted to DSLs either. If you want a new, higher-level general-purpose
    programming language with features seabass doesn't have, you can write it in seabass.

* Seabass is "Domain Complete"

    Disregarding memory limitations (You might have issues compiling a 3 Gigabyte program),
    file size constraints, and implementation bugs, there is at least one seabass program 
    which can generate a given program in another language.
    
## Answers to common concerns people have about programming languages

### Is CBAS fast?

It depends on what you compile CBAS code into. If you decide to compile
Seabass into BASIC then it might not be very fast.

If you're compiling to C the answer is "Yes".

Cbas translates virtually 1-to-1 to C. The higher level capabilities of the language that don't
exist in C are compiled into equivalent C code (Assuming no compiler bugs).

Lower-level capabilities that seabass has which C does not have (tail calls, dispatch tables) rely
either on an optimizing compiler (for tail calls) or C compiler extensions (for dispatch tables).

If there is any feature of C not implemented in Seabass, rest assured, it can still
be accessed by writing inline C (There is a special tool just for it, @inline_C!)

### Is this language portable?

Yes. The compiler itself requires a 64 bit environment but programs written
in the language could target anything from a microcontroller to a modern-day 
x86_64, ARM, or RISC-V machine.

### Do you have a standard library?

For user-mode applications on POSIX-compliant (and posix-like) systems, yes.

It supports most of the useful parts of the C standard library, or supplements
them, where appropriate.

It is currently in development but is already sufficient to write simple
unix applications.

I have plans to extend the library to support most of posix.

### Does the language scale?

Because Cbas is a metaprogramming language, it scales almost infinitely. If the base
level language does not provide the set of abstractions needed for your work, you can
write new ones.

Writing large, long-lived maintainable codebases which require continual modification
should be easier in CBAS than C, C++, or Rust.

### Does the language have inline assembler?

Yes. The `asm` statement allows you to write code inside of a string literal which will be
emitted directly to the target code file. In practice, this means you can write C code
inside of a string literal and have it bake into your program.

### Can I get syntax highlighting for it?

I have written a syntax highlighting file for my editor of choice, micro, however
you should be able to fairly trivially modify a syntax highlighting file written
for C to highlight Cbas code.

### Is Seabass garbage collected or etc?

No. The memory management scheme is identical to C, but with automatic constructors
and destructors.

You can implement your own memory management schemes in higher level languages if you
wish.

### Does seabass support alignment?

Structs can be given an alignment. Simply enter an integer value somewhere
inside the struct definition, and that will be its alignment.

### Does Seabass support long doubles?

In the TOC converter? Officially, no, but you can write
inline C. `math.h` is included by default, so you should
have access to all of the standard math library functions
too.

### Does seabass support unions?

Yes. Type the word `union` anywhere in a struct/class definition. It
will be declared as a union.

### Are there any "gotchas"? 

I don't really think so. 

Interop with C is fairly trivial, and examples are provided.

Compiletimes are typically much slower than C, however the
increased development speed and quality should more than make
up for it.

### Is it possible to write my own tokenizer?

Yes. Using `__builtin_read_file` you can get the contents
of any file given a path at compiletime.

You can then tokenize this however you see fit.

With seabass, it is therefore possible to write your own
compiletime compilers, all the way from tokenization
to code generation. What other language do you know which
can do that?

### Can I have global target code blocks?

Of course! The C code generator already uses this feature to do the standard includes.

## ***For the love of Programming and Programmers***

In the public domain, so that your joy may be full.

https://github.com/gek169/seabass

https://codeberg.org/gek/seabass

All Glory to the Lord Jesus Christ, whose blessings I do not deserve.

# MANIFESTO

For the maximum utilization of Man's God-given talents.

# Theories / Design Principle

## Arbitrary Syntax

A wide variety of notations are needed not only to enable a programmer to best
express his ideas in code, but also to help that programmer think about programming.

To put it another way, high-level computer programming languages perform two
distinct functions for an individual programmer, intellectually:

1. They work in terms of concepts which mimick his thinking. The language reads
    much like he thinks.

2. They help him create new ideas and understand solutions to his problems. He thinks
    in terms of higher-level building blocks because the language provides them.
    
You should recognize these two as a cycle. A language can either mimick a programmer's
existing thinking, or help him generate new ways of thinking about his code.

Thus, new programming notations literally increase a programmer's raw programming
skills in the mind, in addition to enabling faster handiwork because the language 
"feels" natural to his mind.

I believe this cycle - inventing an abstraction, learning to use the abstraction, becoming
a better thinker because of the abstraction - is a general mode for self-improvement as well
as productivity increase. It utilizes a man's mind the best that a purely text-based programming
language can.

It is thus necessary to be able to create arbitrary abstractions and use them in an existing
codebase with as little effort as possible. You must be able to think up entire new programming
paradigms or languages and then implement them the same day.

Very few languages I know of are capable of this in a pragmatic sense, and especially not C or
C++, the former of which has been my language of choice for years.

## Arbitrary Output

It is not enough to simply allow arbitrary syntaxes to be written. In order to make a truly
portable language, it must be possible to write your own code generators.

This makes it so that you don't have to go through the hassle of installing a new toolchain
just because you want to write code for an unusual platform.

Want to write code for the Dreamcast?

Want to compile your game to webassembly? 

Your employer says you have to write code in a language you don't want to?

Write (or get) a code generator. Bam. Done.

## Friendly

I disdain Rust and C++ for their obtuse language design decisions that make those
languages difficult to understand. I find languages like Lua and Basic to be far more comfortable
and pleasing to my soul.

So instead of making design decisions like C++ or Rust, I chose to do things more like
Lua and BASIC. I wanted a language that was approachable, powerful, and portable.

## Free

I want my tools to belong to me- not some corporation, not the FSF, not GNU, me.

So Seabass is public domain- CC0. Every person who downloads the compiler owns it
all for themselves. You will never have GPL lawyers coming after you because
you forgot to publish source code for a patch.

Seabass is my gift to you in perpetuity.

Don't think of it as my language- think of it as your language.

## Easy to Compile and Build

I hate all existing build systems. Here is a 1 minute tutorial for building Seabass from source and running
the unit test:

```
# From the root directory of the project
cc -O3 *.c -o cbas
cd tests
../cbas vm_test.cbas
```

If it tells you "codegen_main executed successfully" it worked.

Put `cbas` somewhere like `/usr/local/bin/` (Somewhere on your PATH
so you can call it by name)

Want to install the standard library and compile a program to C? Here's how you do it.

Navigate to the root of the repository.
Run these commands with administrator privileges to install the standard library:
```
# From the root of the repository....
mkdir -p /usr/include/cbas/
# This just copies the contents of library/ to /usr/include/cbas/
cp -a library/. /usr/include/cbas/
```

Then compile the fibonacci number program like so:

```
cbas tests2/toc_fib_example.cbas
# the linker flags are needed to get the standard library stuff,
# and also making signed-integer overflow defined.
cc -O3 auto_out.c -lm -lpthread -fwrapv -o fib
# use your shiny new fibonacci number program
./fib 20
```

Or the directory listing program like this:

```
cbas tests2/dirlist.cbas
cc -O3 auto_out.c -lm -lpthread -fwrapv -o dirlist
./dirlist .
```



Never use Cmake again!


# My Blessing to you

This software was written for the public domain using the
undeserved blessings of our Lord Jesus Christ. I am a
sinner, unworthy of recognition or appreciation.

For you, Programmers:

This is the blessing wherewith I bless you:

You are made in the Image of God, designed to have dominion over
the works of God's hands. May this piece of software increase
the joy you find in the labor of your hands.

May your adventures be as grand as your imagination can entail.

________________________________________________________________________

# Attribution Request

As stated before, Seabass is a public domain work. It does not require
you to make any attributions or include duplicate license files in
copies or derivative works that you distribute, nor does it impose
any "copyleft" requirements.

However, I have personal requests for anyone using Seabass or deriving
from it:

Please do not remove the dedications to our Lord Jesus Christ. I am fine
with you removing my name or initials, but the project is dedicated to
Christ. If this offends you, it is my request that you simply avoid my
software rather than defile it by removing the dedication and mentions
of our Lord Jesus Christ.

The only exception is if you have a real technical reason (i.e. porting
Seabass to a low-memory platform) for removing the dedication. In which
case you should simply move the dedications into a separate file.

None of my work would be possible without the undeserved blessings of
God our Father and our Lord Jesus Christ. I'm not a good christian, 
mentioning him in my work is all that I can really do. I seek to obey
the command to acknowledge God in all my works, so that he will bless
them.

Again, none of this is a license requirement. It's what you will do if
you care about me or remembering God. Whether you appreciate my work
will be shown by whether or not you preserve the mentions of Jesus
in Seabass's source code, C code generator, help screen, and manpages.

