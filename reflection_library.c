

const char* license_text = 
#include "license_text.h"
;


const char* reflection_library_contents = 

#include "reflection_library.h"

;


const char* builder_library_contents =

#include "builder_library.h"

;


const char* builder_example_contents = 

"\n\n\n#include \"cgbuilder.hbas\"\n\n\n\n@mkbldr [\n\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\t\tmake_cgints\n\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\t@initqtok \"codegen\"\n\t@pushqtok \"int\"\n\t@pushtok [inlist:dupe()]\n\t@pushqtok \";\"\n\tinlist = inlist.right;\n\t//if(inlist == cast(cgstrll*)0) return retval; end\n\t@ifnull inlist \n\t    return retval; \n\tend\n\tgoto top;\n\t:top\n    \t@pushqtok \"codegen\"\n    \t@pushqtok \"int\"\n    \t@pushtok [inlist:dupe()]\n    \t@pushqtok \";\"\n    \tinlist = inlist.right;\n    \t@ifnull inlist \n    \t    return retval; \n    \tend\n\tgoto top;\n\treturn retval;\n]\n\n\n@mkbldrn test_bldrn 3 [\n    @initqtok \";\"\n    __builtin_puts(\"This is what I received:\");\n    inlist:debug_print();\n    return retval;\n]\n@test_bldrn [five men] [with swords] [holding spears]\n@test_bldrn five [with swords] [holding spears]\n\n@mkbldrn test_bldrn_0 0 [\n    @initqtok \"codegen\"\n    @pushqtok \"int\"\n    @pushqtok \"banana\"\n    @pushqtok \";\"\n    return retval;\n]\n\n\n@make_cgints [hairy cat dog meow]\n\n@test_bldrn_0\n\nfn codegen codegen_main():\n    //__builtin_exit(1);\n\thairy = 3; cat = hairy; dog = cat;\n\tmeow = cat + 1;\n\tif(hairy) __builtin_puts(\"Hairy is real!\"); end\n\tif(dog == cat) __builtin_puts(\"So is dog and cat!\"); end\n\tif(meow == cat + 1) __builtin_puts(\"Mrow!\"); end\n\t__builtin_puts(\"Thank you for testing 'bldr', come again soon!\");\n\t\n\tbanana = 5;\nend\n"

;


const char* oop_syntax_example = 

"\n"
"\n"
"/*\n"
"    Small program to demonstrate object oriented programming in Seabass.\n"
"*/\n"
"\n"
"\n"
"class myClass\n"
"    noexport //fancy keyword that basically means this type is not meant for the target.\n"
"    int a\n"
"    char* b\n"
"end\n"
"\n"
"codegen int a = 25;\n"
"\n"
"method codegen myClass.ctor():\n"
"    this.a = a++;\n"
"    char[500] buf\n"
"    __builtin_itoa(buf, this.a);\n"
"    this.b = __builtin_strdup(buf);\n"
"    __builtin_puts(\"I have been constructed! My string is...\");\n"
"    __builtin_puts(this.b);\n"
"end\n"
"\n"
"method codegen myClass.dtor():\n"
"    __builtin_puts(\"Goodbye! My string was...\");\n"
"    __builtin_puts(this.b);\n"
"    __builtin_free(this.b);\n"
"    a--\n"
"end\n"
"\n"
"method codegen myClass.print():\n"
"    __builtin_puts(this.b)\n"
"end\n"
"\n"
"\n"
"\n"
"\n"
"fn codegen what(): if(1) end end\n"
"\n"
"fn codegen codegen_main():\n"
"\n"
"    i64 i\n"
"    for(i = 0, i < 10; i++)\n"
"        myClass qq\n"
"        myClass qq2\n"
"        myClass qq3\n"
"        myClass qq4\n"
"        qq3.print()\n"
"        if(i == 3) continue end\n"
"        qq2.print()\n"
"    end\n"
"    \n"
"    if(1)\n"
"        //demo move...\n"
"        myClass qq\n"
"        myClass qq2\n"
"        __builtin_free(qq.b); //ensure we don't leak memory...\n"
"        qq := qq2;\n"
"        qq2.b = __builtin_strdup(\"I seem to have lost my old string...\");\n"
"        qq.print();\n"
"        qq2.print();\n"
"    end\n"
"    \n"
"    return\n"
"\t\n"
"end\n"
"\n"
"\n"
"\n"
"\n"
""

;
