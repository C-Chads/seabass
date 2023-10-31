______________________________________________________________
# SEABASS
______________________________________________________________

![logo](logo.png)
______________________________________________________________

Seabass is a public domain, self-retargeting, self-extending, 
self-modifying metaprogramming language and general-purpose 
metaprogramming tool.

Let's talk about what that means, shall we?

## Public Domain

Seabass is a public domain dedication, so that your joy
may be full. No attribution required.

## Language

Seabass is, fundamentally, a programming language. You can write
software with it. The syntax of the language is fairly unique,
however it is close cousins with Lua, BASIC, and C.

## Tool

Seabass is also a programming tool. Because of its nature, it is
useful for more than just writing programs in its own language, 
but also implementing your own programming languages.

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

Here is a minimal complete example program, which you can find under 'library/toc_fib_example.cbas'

```c
#include "toc.hbas"
#include "bit64.hbas"
#include "seabass_stdlib.hbas"


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
        uint pow
        pow = 1;
        while(value/pow >= 10)
            pow = pow * 10; 
        end
        /*found the highest power of 10*/
        while(pow)
            uint temp
            temp = value/pow; /*if we had the number 137, we would have gotten
            100 as our pow. We now divide by pow to get the highest digit.*/
            
            dest[0] = (temp + ('0')); dest++;
            
            value = value - temp * pow; /*Get rid of the highest digit.*/
            
            pow = pow / 10 /*Divide pow by 10.*/
        end
    end

    :ending
    dest[0] = 0

    return;
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

### Is this language portable?

Yes. The compiler itself requires a 64 bit environment but programs written
in the language could target anything from a microcontroller to a modern-day 
x86_64, ARM, or RISC-V machine.

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

### Can I have global target code blocks?

Of course! The C code generator already uses this feature to do the standard includes.

## ***For the love of Programming and Programmers***

In the public domain, so that your joy may be full.

https://github.com/gek169/seabass

https://codeberg.org/gek/seabass

All Glory to the Lord Jesus Christ, whose blessings I do not deserve.

# MANIFESTO

For the maximum utilization of Man's God-given talents.

# DOCUMENTATION

see `doc.html` for the official "readable" documentation (in development)
or run `cbas -m help` to view the manual pages.

# Theories / Design Principle

## THEORY 1: YOU MUST BE ABLE TO CREATE ARBITRARY SYNTAXES

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

## THEORY 2: YOU MUST BE ABLE TO OUTPUT CODE TO ANYTHING

It's nice to have a compiler for the computer you have right now, but what if you get a new one
which doesn't have a compiler for it?

You'll need to retarget your toolchain. First, you write a cross compiler, then you port the
toolchain onto the target.

I will discuss the solution to this problem and the next one shortly...

## THEORY 3: YOU MUST BE ABLE TO BYPASS THE AST OF THE LANGUAGE AND OUTPUT WHATEVER

If a target architecture appears which is incompatible with the metaprogramming language's AST
representation, it must be possible to simply cut out the middle man and write your own.

This is more work than should ordinarily be necessary, of course, but nonetheless the circumstances
may arise.

If a language is restricted so that it does not allow bypassing the AST of the language and generating
whatever to output, then it can never write code for a platform which is fundamentally dissimilar.

For instance, if that metaprogramming language has pointers, but you desire to compile code for a 
target that does not have pointers (Javascript, for example) then it would be impossible to use
the metaprogramming language to develop directly for that platform, because the metaprogramming 
language's AST has pointers baked into it.

The solution to 2 and 3 is to allow for a program to implement its own code generator and keep track
of state beyond the compiler's internals.

This enables a metaprogramming language to target any conceivable platform, as a universal
development platform.

## How does SEABASS work under the hood?

Seabass is principally an interpreted language-
running the "cbas" main command on program
source code won't actually generate any
code output by default.

Here is the procedure in rough detail:

1. The source file is read in.
2. The source file is lexed, expanding includes
3. Explicit macro operations (#define) are handled
4. Some simple formatting of the tokens is done
5. Parsing...
	* Symbols at global scope are defined as either being
	for the target (no label) or "codegen" meaning they
	are meant to be used at compiletime. Everything is added
	into the AST. Functions are parsed and built as well.
	* Functions parsed are validated and made fully inter-
	connected by seabass.
	* By using the metaprogramming operator (@) you can
	call any codegen function and have it "take over" 
	parsing until that function returns.

    	This happens DURING PARSING, so it is literally possible
    	for a function you defined earlier in the file to be called
    	WHILE THE FILE IS BEING PARSED.

    	This function is given full access to the AST and token
    	stream, allowing you to modify the input code (token stream-
    	not the actual file itself.)
    	as well as the AST and potentially even already-parsed
    	codegen (compiletime) code.
	
6. Code generation
	* seabass looks for a function you defined called
	"codegen_main" somewhere in the unit which is a "codegen"
	(aka compiletime) function. This function could do
	anything, but its purpose is to take the AST (all global
	symbols not defined codegen) and turn them into target
	code.

## What does SEABASS look like?

Well, it's a metaprogramming language. So, by definition, it looks however you want it to!

The project has advanced more and I can firmly say with authority...

* The base level language is a mixture of Lua, Basic, C++, and C. It supports
simple object oriented programming without inheritance.

* Custom notations vary wildly depending on use-case. There is a running
theme in my work that I tend to surround my custom notation code in square brackets `[]`
for convenience of parsing.

Notable base-level language syntax tidbits which make it "different" from C:

* SEABASS has support for automatic constructors and destructors. `ctor` and `dtor` taking in
    zero arguments and returning void each. These methods are not automatically used on arrays of structs, only
    single local variables (not pointers either).
    
    Constructors and destructors for structs are never auto-generated, but if you explicitly define them, they
    will be used whenever you declare variables of those types in a scope.

* Semicolons are used to terminate statements, however most statements do not require a terminating semicolon. For instance,
    continue, break, if/elif/else/end, while/end, for/end, and goto all do not require semicolons. Variable declarations only
    need a semicolon if they contain an expression statement (an assignment) and expression statements only need a semicolon
    if they are not trivially followed by a non-expression statement. This saves keystrokes.

    
* Methods exist in the language with both lua and C++ style syntaxes, myClass:myMethod() or myClass.myMethod() I personally
    prefer the latter, but the former is more explicit. Function pointers do not exist in the type system (they are just byte*)
    so there is never the possibility of ambiguity, however someone coming from C might be confused.
    
    Do not let the object oriented programming syntax sugars fool you, though. SEABASS is pretty much just C but friendlier.

* Switch generates dispatch tables and doesn't do bounds checking, and uses labels.

    Like this:

    ```
        :is_zero
            puts("Hey!"); goto done
        
        
        switch(2) is_zero, is_one, is_two;
        
        :is_one
            puts("You!") goto done
            
        :is_two
            puts("Listen!") goto done
            
        :done
            puts("Did you see it?");
        
    ```

    It would be undefined behavior to have switch(3). This was done so that switch
    in Seabass would be as fast as possible.
    
* Structs cannot exist as rvalues or lvalues in expressions, only pointers. Therefore, there is a
    dedicated operator for copying between pointers, called the MOVE operator, `:=` which "moves"
    the contents of one pointer into another. This also (somewhat frustratingly...) means that
    indexing an array of structs requires adding to the base pointer rather than indexing with
    brackets, as bracket indexing implies dereferencing. It also means you can't pass or receive
    structs by value with functions.

* You cannot take the address of a variable unless it is a member of an array or struct. This allows
    a code generator in seabass to know beforehand which variables are eligible for putting in registers
    without analyzing the source code to see if a variable ever has its address taken. You can take the
    address of globals with `getglobalptr`, though.

* Dereferencing a pointer is always done with brackets, `[index]`

* Macros are gimped. Define does not allow arguments. Honestly, you don't even need macros because of the
    metaprogramming operator...
    
* the function pointer syntax is much easier to deal with. `getfnptr` and `callfnptr`. There's no syntax
    for defining a function pointer, because they don't exist as types. `callfnptr` takes a template function
    to "steal" its prototype from, then the expression evaluating to a `byte*` which is the function pointer,
    then the arguments to the function.
    
    `callfnptr[myTemplateFn](myFunctionPointerVar)(arg1, arg2, arg3);`

* Rust-style return types are used. `fn myFunction(int arg1, i32 arg2, slong arg3)->long: return arg1+arg2+arg3 end`

* Seabass uses numerous "duplicates" of the default keywords and some operators to help people with bad memories. if
    you don't remember a keyword, it's very likely a synonym will work (instead of goto, you can write jump).
    
    I have pondered whether this is an anti-feature (Who would expect `sqword` to be a keyword for 
    a signed 64 bit integer?) and I have removed most of the aliases for streq and strneq and eq/neq.
    
    I believe the aliases add "flavor" to the language. It feels friendlier this way.
    
    Notably, `===` is equivalent to `==`, and `eq` and `neq` can be typed instead of
    the other way.
    
* `cast` is a keyword. C-style casts are allowed, but you can also do `cast(type)` to do a cast.

* Labels for goto and switch put a colon BEFORE the label name. This is needed to prevent an
    ambiguity in the language..

## Project state

(UPDATED OCTOBER 29th 2023)

Seabass is advancing rapidly. If the blessings of God continue, and I
manage to avoid being lazy, the project should reach a "completed" state within
a few months.

### PROJECT GOALS

 - [X] One complete implementation in C99 
 
 - [X] A compiletime reflection library
 
 - [X] A C code generator
 
 - [ ] A standard library
 
    - [X] basic file I/O
    
    - [X] Multithreading
    
    - [ ] Networking
    
    - [ ] Math
    
    - [ ] Algorithms

 - [X] Syntax highlighting files

 - [ ] A complete metaprogramming library
 
    - [X] The Builder
    
    - [X] The Worksheet
    
    - [X] Token Manipulation Tools
    
    - [ ] Language design tools
    
        - [X] cgrdparse- compiletime recursive descent parser generator
        
            - [ ] Parser Rule Templates- Common patterns in recursive descent parsing.
        
        - [ ] cg_uast- the Universal Abstract Syntax Tree (Unfinished)
        
    - [X] Tbas- Templated Seabass
    
    - [ ] System Archetypes
    
    - [ ] Datastructure Implementation Language (DIL)

 - [ ] A standard template library
    
 - [ ] A self-hosted implementation (MILESTONE- READY FOR PRODUCTION)
 
 - [ ] Multiple custom high-level general-purpose languages to replace languages like C++ and Rust.
 
    - [ ] CBAS++ (New features include...)
        
        - [ ] Actors- a Multithreaded programming model
        
        - [ ] Generics (C++ templates, but type-only)
        
        - [ ] Templated code
        
        - [ ] Everything Cbas has and more
        
        - [ ] Automatic creation of pass-by-value wrappers.
        
    - [ ] A functional language
    
    - [ ] A rust-like borrow-checked language.
 
 - [ ] A large set of domain specific languages for solving specific programming problems

 - [ ] SDL2 port (MILESTONE- APPLICATION-READY)

## ROADMAP (What order do I make things in?)

* cgrdparse + cg_uast tools

I need more tools for writing recursive descent parsers that generate ASTs using the
UAST. To do this, I want to write a toy expression parser and have it parse into the
UAST. This will be used as the test-bench for the tools. Needed tools:

    * Common UAST patterns- such as...
    
        * Add child
        
        * maintaining a stack
    
    * Parser authoring helpers
    
        * binop rule templates - do some rule, 
        resulting in a new node on top of the parse
        stack, then check for a sequence of tokens,
        and if they're present, parse the RHS.
        Different binop rule templates will provide
        different chaining rules (left-left-folding,
        right node insertion, etc)
        
        * sequence rule templates - do some
        rule, then check for a sequence continuator
        or a sequence terminator. If the sequence continuator
        is present, continue the sequence. Sequence
        terminator terminates sequence. What happens
        when neither of these is present is definable.
        Whether or not the terminator is consumed is
        also definable.
        
        * Nested pair parser- check for an opening 
        token sequence. If it is present, consume
        it and execute some rule. When it returns,
        check for the presence of the terminating
        symbol.
        
        * Error- Need to standardize how errors
        are handled by `cgrdparse` parsers. I
        figure there will be a standard `error`
        rule in all parsers, that you `go error`
        to...
    
    * Parser Generator

* Self hosting

Seabass needs to be ported into itself so that work
on the compiler is easier to verify as correct and
easier to do.

Once that occurs, all future development on Seabass
will move to that repository and this compiler
will be archived. Future bootstraps will use
the latest build-to-C.

This update should also remove Seabass's 64-bit-only
requirement and possibly even add support for 128
bit integers into the language.

* Data structure templates

I need common data structures such as `vector` and
`list` and `map` available at my finger tips, and I
need it without any hassle. It's what I miss most from
C++. I could theoretically already template any data
structure I want by using `cgwksht` however this would
not allow for inline usage ala `vector<int>`.

* Cbas++

I need a language on top of Seabass which provides me with
everything C++ has, except readable. Everything from automatic
type conversions to operator overloading and of course templates.

It will also be a metaprogramming language.

* DIL / System Archetypes

My visions for Higher order programming are what keep me inspired.

In order to make those dreams a reality, I need a programming language
specifically for writing systems and data structures.

* SDL2 port

I want to be able to write games and graphical applications using
Seabass- so I would like to properly port SDL2 into the language.

* Text editor

I want to write the coolest programming editor ever.
    
	
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
________________________________________________________________________
________________________________________________________________________
________________________________________________________________________
# If you like it?

I believe that this project, like many others I've worked on,
is a gift from the Lord Jesus Christ. This is a labor of love
and passion which I have been given.

Without the blessings of God, this project would be impossible.
If I have done any good thing, it is because the LORD prepared
it for me to do. I have no greatness of my own, but only that
which the LORD put into my own hand.

### If you appreciate my software, Thank God for letting me do it.

As stated before, none of what I do would be possible without
the blessings of God.

If it seems like I'm repeating myself, it's because I'm afraid
of his abasement. I can't afford to be abased by him.

I require time, motivation, health, good ideas, food, water,
sleep, and a variety of other things to write software.

All of that could evaporate a moment's notice if the LORD 
ceases to bless me.

### If you're not a believer?

Enjoy the software. Have fun with it.

May the truth himself come upon you, bringing everlasting joy
and eternal life, so that you might be saved. 

None of my work would be possible without His blessings.

If there is anything you take away from the meager works of
my hands, I hope it is a better view of Christ, who deserves
all the glory and praise. Go to a secret place and thank him
if you truly appreciate my work.

Remember the gospel:

Jesus died on a cross, was buried, rose from the dead the third day
as recorded in the scriptures, and was seen and touched by many.

Believe this truth and you will be saved.

# Building

Check the `Makefile` to see how to build the project.

The project has quite minimal dependencies, so you shouldn't
have any problem compiling it.

# Learning / Quickstart

the `cbas` program includes built-in manual pages
to help you learn the language. They are incomplete,
but several runnable examples are contained within.

They don't document the metaprogramming library, only
the base level language and `cbas` tool. Writing
docs for the metaprogramming library is a TODO.

To begin viewing these manpages, do `cbas -m help`

## Platform specific stuff

The compiler itself has been modified not to use any *nix specific functionality, so
the compiler should be portable to non-POSIX platforms (such as Windows).

However, the stdlib implementation (for compiling Seabass interop with C) currently
implements threads using `pthreads`. Furthermore, this implementation does not
work on ...Fruit... devices.

## Implementation Limitations

Seabass assumes that the codegen environment is 64 bit. You can
write code for a non-64 bit target, but the compiler itself must
live on a 64 bit host.

When Seabass is eventually ported to itself, I plan to address
this by allowing you to define the compiletime environment.

# Enjoy Programming!

Repository note: Reinitialized on October 16th
