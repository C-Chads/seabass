
# I want to make a metacompiler library- a set of tools for writing compilers at compiletime.

Part of the joy of seabass will be making new languages to express high-level
constructs efficiently and succintly- that you can write entire new programming
languages with ease.

This necessitates the existence of efficient tools for manipulating tokens, text,
and abstract syntax trees.

TOOLS NEEDED

1. UAST traversal code- It needs to be easy to write traversals of an instance of the UAST. I figure
    I can use ideas from the AST designer idea (entrance, revisitation, retreat) to specify
    how a traversal happens.
    
2. There are certain "recurring themes" in language design that I want to template...

    Especially things like "list of statements" and infix notation parsing. This system
    should be a universal system for templating the parsing of programming languages
    which are roughly C-like.

    * You can stack-up a recursive descent parser for infix binary expressions by
    using object oriented programming. Each object keeps internal state
    describing what operator it is and then stores more information about
    what lower-level parse rules it tries to apply
    
        NOTE: Incompatible with cgrdparse, most likely a bad idea..
        

THE SOLUTION: IMPROVE / BUILD UPON CGRDPARSE AND TOKMANIP TOOLS.

    CgRDparse is my most powerful parser library tool.

IDEA: SPECIFY A HIERARCHY MANUALLY...

    binary_operator_hierarchy myHierarchy ("=",":="),("*","/"),("+","-"),(parse_prefix)
    
    it should then generate a bunch of `cgrdparse` rules for each
    level of this hierarchy. This should make it very easy to specify
    new languages just by writing out the order of operators you want to parse.

    
