______________________________________________________________
# SEABASS
______________________________________________________________

![logo](logo.png)

# THE SEABASS METAPROGRAMMING LANGUAGE

Seabass (or CBAS) is a small, portable, public domain general-purpose programming language which
compiles into efficient C code.

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
    uint qq = matou((char*)argv[1]); //unsigned->signed conversion is fine here.
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

If that was the end of it, then seabass would be little more than an interesting toy. A syntax
sugar for C.

However, seabass has several "superfeatures" which place it firmly amongst the "heavyweights":

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
    
* You can bypass almost any restriction seabass has.

    If anything about seabass as a language or as a compiler bothers you, it is usually
    possible to override or change it, from syntax annoyances to how commandline arguments
    are parsed.
    
    There are some "hard" limitations of Seabass:
    
    * Obviously, seabass code is limited by the hardware it runs on. If you run seabass code on a slow
        computer, it will run slowly.
    
    * It is not possible to create custom syntaxes within seabass which are whitespace-oriented-
        at least, not without wrapping them in string literals. If your dream was to re-create python
        inside of seabass, then... well... sorry!
    
    * All "codegen" code executes inside Seabass's "AST executor". This execution is not particularly
        fast (it's significantly slower than python). This is only a real problem if you are trying
        to write compiletime virtualization layers.
    
    * the second element of `argv` is reserved by Seabass for the name of the main file
        of the compilation unit.
    
## Answers to common concerns people have about programming languages

### Is CBAS fast?

It depends on what you compile CBAS code into. Cbas is self-retargeting, you see.

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

### Are there any "gotchas"?

Yes, however they are almost entirely bikeshedding. Seabass gives you enormous flexibility and
power with what you can make.

I know of the following real "gotchas" with seabass, mostly surrounding the current compiler implementation:
    
    * The compiler itself assumes that the platform it is running on is a 64 bit system with byte-addressable
        memory. So the compiler itself can't be compiled for retro computers.
        
    * The compiler implementation requires the `realpath` function, which comes from POSIX. This function
        is not available on Windows, so either a supplementary implementation must be provided, or
        the compiler itself must be modified to not require this function.
        
        It is used for handling include statements.
        
    * custom syntaxes in Seabass may not be whitespace-oriented... unless you want to write all your code
        inside of a string literal.
        
    * `codegen` code in seabass is not very fast. it directly executes the AST, not even using a bytecode.
    
        I plan to implement a bytecode when I get around to making seabass self-hosting.
    
    * The compiler greedily over-allocates memory, especially at startup, reserving multiple megabytes.
    
        This shouldn't be a problem for any serious use, however I know it will drive software minimalists
        absolutely bonkers.

There are also some gotchas which are not really gotchas, but seem like they are:
    
    * there are no 128 bit integers or long doubles (10 byte floats). 

        This is not really a gotcha, as it is still possible to write seabass code which uses
        long doubles/128 bit integers by embedding their usage in the `asm` statement.
        
    * Structs/Unions are always passed by pointer, not by value, even if they are small.
    
        You optimization geeks might think this is a serious limitation, 
        however my own testing has revealed that not only is this not true, but the opposite is
        often the case- that seabass's pass-by-pointer convention can force your code
        to be faster.
        
        It have found that it is sometimes faster to pass an 8 byte pointer to an 8 byte struct than
        to pass the struct itself. Yeah.
        
        If you absolutely have to pass large objects by value, you can always subdivide your object
        into 8 byte chunks and pass it as multiple arguments.
        
        (If you are writing a SIMD library, use 'inline')
        
        If seabass's AST (which requires pass-by-pointer conventions for structs) is
        too limiting for you, you can write your own.
        
        So this is not really a gotcha.


## ***For the love of Programming and Programmers***

In the public domain, so that your joy may be full.

https://github.com/gek169/seabass

https://codeberg.org/gek/seabass

All Glory to the Lord Jesus Christ, whose blessings I do not deserve,
yet he loves me so much that I get them anyway. Amen.

# MANIFESTO

For the maximum utilization of Man's God-given talents.

# Theories / Design Principle

see `doc.html` for the official "readable" documentation (in development)
or run `cbas -m help` to view the manual pages.

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

* The base level language is a mixture of C++, C, and Lua. It supports simple object oriented
programming without inheritance, but doesn't automatically write constructors or destructors
(it only calls ones you've already written).

* Custom notations vary wildly depending on use-case. There is a running
theme in my work that I tend to surround my custom notation code in square brackets `[]`
for convenience of parsing.

Notable core language syntax tidbits which make it "different" from C:

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

* The `codegen` environment does not do any short-circuiting for && or ||. It is not right to do pointer validity checks by taking advantage of short circuiting. It's difficult to follow.

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

Seabass is advancing at a modest pace, not as quick as I'd like, in
part because I am doing a lot of theory work. What are the best
metaprogramming paradigms? How do I make metaprogramming as easy
as possible? What are the general principles of metaprogramming?

Seabass has become a minimum viable product for metaprogramming now
that it has a `toc` converter, but it has yet to truly supersede
Rust or C++ in raw programming power. In order to advance to that
point, SEABASS will require a standard template library and a
full reflection of the C standard library. Additionally, I want to
write a higher-level Rust-style language (which, unlike rust, will
be easy for me to read and write).

Seabass is on the war path, but I cannot yet say that it is more 
powerful than the juggernauts.

## The War Path

I am working on Seabass with the intent to make it more powerful than all
other programming languages currently in large-scale production (even lisp).

Here's the general plan:

1. Make metaprogramming in seabass easy

    It has to be possible to create new compilers and metaprogramming tools in seabass with little effort,
    
    so that a person can have an idea and then implement it the same day.
    
2. Create new languages and tools within Seabass with abstractions that supersede those of
    other languages.
    
    I have plans for meta-tools, libraries, and even entire programming languages (Such as OOPSY)
    to write in/for Seabass, which should make using Seabass easy. This will make the language
    more powerful than Rust.
    
    I believe that once OOPSY is finished, i can truly say that seabass is more powerful than
    the "juggernauts" of our time.
    
3. Write documentation and tools for working with Seabass (code and projects) 
    to make it accessible and easy, even for beginners.
    
    Notably, I want to make my own text editor, build system, and probably a project manager.
    
    This will give me a proper IDE for development in Seabass.
    
4. Port Seabass into itself.

    I want seabass and all tools written in it to be self-hosting. The current compiler is
    the bootstrap compiler.
    
    Using the power of seabass's metaprogramming and arbitrary abstractions, I should be
    able to formally verify the correctness of this implementation, whereas the C99
    implementation is virtually impossible to prove anything about.
    
    This would make seabass truly viable for "mission critical" work, such as life support
    systems or aircraft system software.

    
	
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

This code uses a little bit of *nix-specific functionality, specifically
to evaluate paths for included files.

It works on Debian 12 so it should work on whatever distro you're on.

If you're on windows for some reason
then you'll have to replace `realpath` with some windows 
equivalent.

realpath DOES return an owning pointer, by the way.

Seabass assumes that the codegen environment is 64 bit. You can
write code for a non-64 bit target, but for instance, you'll
have to evaluate type sizes yourself. It's not that difficult,
it's just something you'll have to write.

If and when seabass is ported to itself, I may make `cbas` compatible
with other environments. It's a very low-priority goal.

# Enjoy Programming!

Repository note: Reinitialized on October 16th
