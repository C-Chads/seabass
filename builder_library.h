"\n"
"#guard cg_builder_library_DMHSW\n"
"\n"
"#include <cgast.hbas>\n"
"\n"
"/*\n"
"\tTHE BUILDER NAMESPACE\n"
"\n"
"\tParsehooks are sometimes used only for builders.\n"
"\n"
"\tI want to have a special namespace system specifically for builder parsehooks\n"
"\twhich only work for codegen code.\n"
"*/\n"
"\n"
"\n"
"struct cg_builder_hook_namespace_entry\n"
"    noexport\n"
"\tcgstr real //how is it REALLY invoked, the stuff after @?\n"
"\tcgstr good //what is its fake name, the ?\n"
"end\n"
"\n"
"codegen cg_builder_hook_namespace_entry* builder_hook_namespace_entries = 0;\n"
"codegen uptr builder_hook_namespace_nentries = 0;\n"
"\n"
"\n"
"fn codegen builder_hook_namespace_add(cgstr real, cgstr fake):\n"
"\tbuilder_hook_namespace_nentries++;\n"
"\n"
"\n"
"\t//push onto the vector.\t\n"
"\tbuilder_hook_namespace_entries = cast(cg_builder_hook_namespace_entry*)\n"
"\t__builtin_realloc(\n"
"\t\tcast(u8*)builder_hook_namespace_entries, \n"
"\t\tsizeof(cg_builder_hook_namespace_entry) * builder_hook_namespace_nentries\n"
"\t);\n"
"\t\n"
"\t//realloc never fails, remember?\n"
"\t(builder_hook_namespace_entries + (builder_hook_namespace_nentries-1)).real:fromstr(real.s);\n"
"\t(builder_hook_namespace_entries + (builder_hook_namespace_nentries-1)).good:fromstr(fake.s);\n"
"end\n"
"\n"
"fn codegen builder_hook_namespace_query(cgstr fake)->cgstr*:\n"
"\tcgstr* retval\n"
"\tcgstr* testme\n"
"\tu64 i\n"
"\n"
"\t//search the list\n"
"\tfor(i = 0, i < builder_hook_namespace_nentries, i++)\n"
"\t\ttestme = \n"
"\t\t(\n"
"\t\t\tbuilder_hook_namespace_entries + \n"
"\t\t\ti\n"
"\t\t).good;\n"
"\t\tretval = (\n"
"\t\t\tbuilder_hook_namespace_entries + \n"
"\t\t\ti\n"
"\t\t).real;\n"
"\t\tif(fake.s streq testme.s)\n"
"\t\t\treturn retval;\n"
"\t\tend\n"
"\tend\n"
"\treturn cast(cgstr*)0;\n"
"end\n"
"\n"
"\n"
"//THE BUILDER\n"
"//makes new parsehooks which take in a token stream and return a token stream.\n"
"//it actually writes TWO codegen functions:\n"
"//one of them is the thing which is actually invoked.\n"
"\n"
"fn codegen parsehook_bldr():\n"
"\tcgstrll* retval\n"
"\tcgstrll* rwalk\n"
"\tcgstr fname //the wrapper- which is what is invoked.\n"
"\tcgstr fname2 //the thing you actually wrote, which returns to the wrapper.\n"
"\tcgstr fname3 //the original version of the name..\n"
"\tcgstrll* arglist //the code for the thing you actually wrote.\n"
"\n"
"\tu64 linenum\n"
"\tu64 colnum\n"
"\tchar* filename\n"
"\n"
"\tif(cast(uptr)cg_peek() == 0)\n"
"\t\t__builtin_puts(\"ERROR! bldr invoked at end of file.\");\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\n"
"\tfilename = cg_peek().filename;\n"
"\tlinenum = cg_peek().linenum;\n"
"\tcolnum = cg_peek().colnum;\n"
"\t\n"
"\tretval = cast(cgstrll*)0;\n"
"\n"
"\tif(cast(uptr)cg_peek().text == 0)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"ERROR! bldr invoked on empty token.\");\n"
"\t\t__builtin_puts(\"File:\");\n"
"\t\t__builtin_puts(filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\tfname:fromstr(cg_peek().text);\n"
"\tfname3:fromstr(cg_peek().text);\n"
"\t__builtin_consume();\n"
"\tif(1)\n"
"\t\tcgstr f\n"
"\t\tf:fromstr(\"parsehook_\");\n"
"\t\tf:add(fname);\n"
"\t\tfname:free();\n"
"\t\tfname := f;\n"
"\t\tfname2:fromstr(\"cg_inner_\");\n"
"\t\tfname2:add(fname);\n"
"\tend\n"
"\tif(1)\n"
"\t\tcgstr pre\n"
"\t\tpre:fromstr(\"cg_bldr_\");\n"
"\t\t//this is in the builder namespace.\n"
"\t\tif(fname3:prefixed(pre) && (fname3.s strneq pre.s))\n"
"\t\t\t//create builder namespace entry.\n"
"\t\t\tcgstr fakename\n"
"\t\t\tfakename:fromstr(fname3.s + 8);\n"
"\t\t\tbuilder_hook_namespace_add(fname3, fakename);\n"
"\t\t\t//__builtin_puts(\"Registered:\");\n"
"\t\t\t//__builtin_puts(fakename.s);\n"
"\t\t\tfakename:free();\n"
"\t\tend \n"
"\t\tpre:free();\n"
"\t\tfname3:free();\n"
"\tend\n"
"\t//get the arglist.\n"
"\targlist = cg_token_grab_arglist(\"Invalid argument list passed to builder.\");\n"
"    //arglist:debug_print();\n"
"\t//if this is a lone thing...\n"
"\tif(arglist.d != CG_TOK_OBRACK)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"BLDR ERROR! No Arglist!\");\n"
"\t\t__builtin_puts(\"File:\");\n"
"\t\t__builtin_puts(filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\t\n"
"\t//pop the brackets off.\n"
"\targlist = cg_tok_arglist_remove_brackets(arglist);\n"
"\n"
"\t//define the initial function- the inner one.\n"
"\n"
"\tretval = cg_qtok(\"fn\");\n"
"\trwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"codegen\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(fname2.s));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"->\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(arglist);\n"
"\t\t//walk to the end.\n"
"\t\twhile(rwalk.right != cast(cgstrll*)0)\n"
"\t\t    rwalk = rwalk.right;\n"
"\t\tend\n"
"\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\n"
"\t//define the wrapper.\n"
"\trwalk:push_end(cg_qtok(\"fn\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"codegen\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(fname.s));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\n"
"\t\t\n"
"\t\t\t\t\n"
"\t\t//Some place to store the returned list.\n"
"\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\t//fetch the inlist\n"
"\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\t\t\n"
"\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\n"
"\t\t//fetch the inlist\n"
"\t\t//\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\t\t\n"
"\t\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\t\t\n"
"\n"
"\t\t//use the arglist grabber.\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"cg_token_grab_arglist\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\tif(1)\n"
"\t\t\t\tcgstr qq;\n"
"\t\t\t\tcgstr tt;\n"
"\t\t\t\tqq:fromstr(\"\\\"ERROR! Builder \\\"\");\n"
"\t\t\t\t\t//parsehook_\n"
"\t\t\t\t\ttt.s = fname.s + 10; //skip parsehook_\n"
"\t\t\t\tqq:add(tt);\n"
"\t\t\t\ttt.s = \"\\\" has received an invalid arglist\\\"\";\n"
"\t\t\t\tqq:add(tt);\n"
"\t\t\t\trwalk:push_end(cg_qtok(qq.s));rwalk = rwalk.right;\n"
"\t\t\t\tqq:free();\n"
"\t\t\tend\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t//end the assignment to inlist\n"
"\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\t\n"
"\t\t//remove brackets\n"
"\t\t/*\n"
"\t\t\tif(inlist.d == (cast(u8*)14))\n"
"\t\t\t\tinlist = cg_tok_arglist_remove_brackets(inlist);\n"
"\t\t\tend\n"
"\t\t*/\n"
"\t\t\n"
"\t\trwalk:push_end(cg_qtok(\"if\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\".\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"d\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"==\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\t\trwalk:push_end(cg_qtok(\"u8\"));rwalk = rwalk.right;\n"
"\t\t\t\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\t\t\trwalk:push_end(cg_qtok(\"14\"));rwalk = rwalk.right; //the code for OBRACK.\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"cg_tok_arglist_remove_brackets\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\t\t\n"
"\t\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\t\t\n"
"\t\t//call the inner function. Store the result in a variable- writeme\n"
"\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(fname2.s));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\n"
"\t\t//free the inlist\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"freelist\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\n"
"\t\t//push writeme\n"
"\t\trwalk:push_end(cg_qtok(\"if\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"uptr\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\t\t\n"
"\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"cgtok_push_stream\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\t\t\t\t\n"
"\t\trwalk:push_end(cg_qtok(\"else\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"__builtin_puts\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\t//make something creative.\n"
"\t\t\t\tif(1)\n"
"\t\t\t\t\tcgstr qq;\n"
"\t\t\t\t\tcgstr tt;\n"
"\t\t\t\t\tqq:fromstr(\"\\\"ERROR! Builder \\\"\");\n"
"\t\t\t\t\t\t//parsehook_\n"
"\t\t\t\t\t\ttt.s = fname.s + 10; //skip parsehook_\n"
"\t\t\t\t\tqq:add(tt);\n"
"\t\t\t\t\ttt.s = \"\\\" has returned an invalid arglist, it cannot be pushed onto the token stream\\\"\";\n"
"\t\t\t\t\tqq:add(tt);\n"
"\t\t\t\t\trwalk:push_end(cg_qtok(qq.s));rwalk = rwalk.right;\n"
"\t\t\t\t\tqq:free();\n"
"\t\t\t\tend\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\tfname:free();\n"
"\tfname2:free();\n"
"\t\n"
"\tcgtok_push_stream(retval);\n"
"end\n"
"\n"
"\n"
"fn codegen parsehook_bldrn():\n"
"\tcgstrll* retval\n"
"\tcgstrll* rwalk\n"
"\tcgstr fname //the wrapper- which is what is invoked.\n"
"\tcgstr fname2 //the thing you actually wrote, which returns to the wrapper.\n"
"\tcgstr fname3 //the original version of the name..\n"
"\tcgstrll* arglist //the code for the thing you actually wrote.\n"
"\n"
"\tu64 linenum\n"
"\tu64 colnum\n"
"\tu64 nargs\n"
"\tchar* filename\n"
"\n"
"\tif(cast(uptr)cg_peek() == 0)\n"
"\t\t__builtin_puts(\"ERROR! bldr invoked at end of file.\");\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\n"
"\tfilename = cg_peek().filename;\n"
"\tlinenum = cg_peek().linenum;\n"
"\tcolnum = cg_peek().colnum;\n"
"\t\n"
"\tretval = cast(cgstrll*)0;\n"
"\n"
"\tif(cast(uptr)cg_peek().text == 0)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"ERROR! bldr invoked on empty token.\");\n"
"\t\t__builtin_puts(\"File:\");\n"
"\t\t__builtin_puts(filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\tfname:fromstr(cg_peek().text);\n"
"\tfname3:fromstr(cg_peek().text);\n"
"\t__builtin_consume();\n"
"\tif(1)\n"
"\t\tcgstr f\n"
"\t\tf:fromstr(\"parsehook_\");\n"
"\t\tf:add(fname);\n"
"\t\tfname:free();\n"
"\t\tfname := f;\n"
"\t\tfname2:fromstr(\"cg_inner_\");\n"
"\t\tfname2:add(fname);\n"
"\tend\n"
"\tif(1)\n"
"\t\tcgstr pre\n"
"\t\tpre:fromstr(\"cg_bldr_\");\n"
"\t\t//this is in the builder namespace.\n"
"\t\tif(fname3:prefixed(pre) && (fname3.s strneq pre.s))\n"
"\t\t\t//create builder namespace entry.\n"
"\t\t\tcgstr fakename\n"
"\t\t\tfakename:fromstr(fname3.s + 8);\n"
"\t\t\tbuilder_hook_namespace_add(fname3, fakename);\n"
"\t\t\t//__builtin_puts(\"Registered:\");\n"
"\t\t\t//__builtin_puts(fakename.s);\n"
"\t\t\tfakename:free();\n"
"\t\tend \n"
"\t\tpre:free();\n"
"\t\tfname3:free();\n"
"\tend\n"
"    if(cg_peek().d != CG_TOK_INT_CONST)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"BLDRN ERROR! No nargs!\");\n"
"\t\t__builtin_puts(\"Syntax is @bldrn myBldr_with_3_args 3 [code();]\");\n"
"\t\t__builtin_puts(\"File:\");\n"
"\t\t__builtin_puts(filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\t\n"
"\tnargs = __builtin_atoi(cg_peek().text);\n"
"\t__builtin_consume(); //eat it.\n"
"\t//get the arglist.\n"
"\targlist = cg_token_grab_arglist(\"Invalid argument list passed to builder.\");\n"
"\n"
"\t//if this is a lone thing...\n"
"\tif(arglist.d != CG_TOK_OBRACK)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"BLDR ERROR! No Arglist!\");\n"
"\t\t__builtin_puts(\"File:\");\n"
"\t\t__builtin_puts(filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\t\n"
"\t//pop the brackets off.\n"
"\targlist = cg_tok_arglist_remove_brackets(arglist);\n"
"\n"
"\t//define the initial function- the inner one.\n"
"\n"
"\tretval = cg_qtok(\"fn\");\n"
"\trwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"codegen\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(fname2.s));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\tif(nargs > 0)\n"
"\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\tend\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"->\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(arglist);\n"
"\t\t//walk to the end.\n"
"\t\tfor(0, rwalk.right != cast(cgstrll*)0, rwalk = rwalk.right)\n"
"\t\t\t;\n"
"\t\tend\n"
"\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\n"
"\t//define the wrapper.\n"
"\trwalk:push_end(cg_qtok(\"fn\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"codegen\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(fname.s));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\n"
"\t\t\n"
"\t\t\t\t\n"
"\t\t//Some place to store the returned list.\n"
"\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\t\n"
"\t\tif(nargs > 0)\n"
"    \t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"    \t\tif(nargs > 1)\n"
"        \t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"        \t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"        \t\trwalk:push_end(cg_qtok(\"inlist2\"));rwalk = rwalk.right;\t\t\n"
"    \t\tend\n"
"        end\n"
"\t\t//fetch the inlist\n"
"\t\t//\n"
"\t\tif(nargs > 0)\n"
"    \t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\t\t\n"
"    \t\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\t\t\n"
"\n"
"    \t\t//use the arglist grabber.\n"
"    \t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"    \t\t\trwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"    \t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"    \t\t\t\trwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"    \t\t\t\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"    \t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"    \t\t\t\n"
"    \t\t\trwalk:push_end(cg_qtok(\"cg_token_grab_arglist\"));rwalk = rwalk.right;\n"
"    \t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"    \t\t\tif(1)\n"
"    \t\t\t\tcgstr qq;\n"
"    \t\t\t\tcgstr tt;\n"
"    \t\t\t\tqq:fromstr(\"\\\"ERROR! Builder \\\"\");\n"
"    \t\t\t\t\t//parsehook_\n"
"    \t\t\t\t\ttt.s = fname.s + 10; //skip parsehook_\n"
"    \t\t\t\tqq:add(tt);\n"
"    \t\t\t\ttt.s = \"\\\" has received an invalid arglist\\\"\";\n"
"    \t\t\t\tqq:add(tt);\n"
"    \t\t\t\trwalk:push_end(cg_qtok(qq.s));rwalk = rwalk.right;\n"
"    \t\t\t\tqq:free();\n"
"    \t\t\tend\n"
"    \t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"    \t\t//end the assignment to inlist\n"
"    \t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"    \t\ti64 nargs_left\n"
"    \t\tnargs_left = nargs-1;\n"
"    \t\tfor(0, nargs_left, nargs_left--)\n"
"                //grab an additional argument.\n"
"                      //fetch the inlist\n"
"                      rwalk:push_end(cg_qtok(\"inlist2\"));rwalk = rwalk.right;\t\t\n"
"                      rwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\t\t\n"
"\n"
"                      //use the arglist grabber.\n"
"                      rwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"                          rwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"                          rwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"                              rwalk:push_end(cg_qtok(\"cgstrll\"));rwalk = rwalk.right;\n"
"                              rwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"                          rwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\n"
"                          rwalk:push_end(cg_qtok(\"cg_token_grab_arglist\"));rwalk = rwalk.right;\n"
"                          rwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"                          if(1)\n"
"                              cgstr qq;\n"
"                              cgstr tt;\n"
"                              qq:fromstr(\"\\\"ERROR! Buildern \\\"\");\n"
"                              //parsehook_\n"
"                              tt.s = fname.s + 10; //skip parsehook_\n"
"                              qq:add(tt);\n"
"                              tt.s = \"\\\" has received an invalid arglist\\\"\";\n"
"                              qq:add(tt);\n"
"                              rwalk:push_end(cg_qtok(qq.s));rwalk = rwalk.right;\n"
"                              qq:free();\n"
"                          end\n"
"                          rwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"                      //end the assignment to inlist2\n"
"                      rwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"                      //push inlist2 onto inlist.\n"
"                      rwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\"push_end\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\"inlist2\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"                      rwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"            end\n"
"\n"
"        end\n"
"\n"
"\t\t//call the inner function. Store the result in a variable- writeme\n"
"\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(fname2.s));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t    if(nargs > 0)\n"
"\t\t    rwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\t\tend\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\n"
"\t\t//free the inlist\n"
"\t\tif(nargs > 0)\n"
"    \t\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\"freelist\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"    \t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"        end\n"
"\t\t//push writeme\n"
"\t\trwalk:push_end(cg_qtok(\"if\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"cast\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"uptr\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\t\t\n"
"\t\t\n"
"\t\t\trwalk:push_end(cg_qtok(\"cgtok_push_stream\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\trwalk:push_end(cg_qtok(\"writeme\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\t\t\t\t\n"
"\t\trwalk:push_end(cg_qtok(\"else\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"__builtin_puts\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\t\t\t//make something creative.\n"
"\t\t\t\tif(1)\n"
"\t\t\t\t\tcgstr qq;\n"
"\t\t\t\t\tcgstr tt;\n"
"\t\t\t\t\tqq:fromstr(\"\\\"ERROR! Builder \\\"\");\n"
"\t\t\t\t\t\t//parsehook_\n"
"\t\t\t\t\t\ttt.s = fname.s + 10; //skip parsehook_\n"
"\t\t\t\t\tqq:add(tt);\n"
"\t\t\t\t\ttt.s = \"\\\" has returned an invalid arglist, it cannot be pushed onto the token stream\\\"\";\n"
"\t\t\t\t\tqq:add(tt);\n"
"\t\t\t\t\trwalk:push_end(cg_qtok(qq.s));rwalk = rwalk.right;\n"
"\t\t\t\t\tqq:free();\n"
"\t\t\t\tend\n"
"\t\t\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\t\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"end\"));rwalk = rwalk.right;\n"
"\tfname:free();\n"
"\tfname2:free();\n"
"\t\n"
"\tcgtok_push_stream(retval);\n"
"end\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"/*\n"
"\tBuilder namespace wrapper.\n"
"\n"
"\tused exactly like builder, but automatically detects\n"
"\tusage of stuff in the builder parsehook namespace.\n"
"*/\n"
"\n"
"@bldr mkbldr [\n"
"\tcgstrll* retval\n"
"\tcgstrll* inlist_dupe\n"
"\tcgstrll* following\n"
"\tcgstrll* walker\n"
"\n"
"\tinlist_dupe = inlist:dupell();\n"
"\tfollowing = inlist_dupe.right;\n"
"\tinlist_dupe.right = cast(cgstrll*)0;\n"
"\n"
"\tif( following == 0)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"MKBUILDER_ERROR\");\n"
"\t\t__builtin_puts(\"Invoked mkbldr with null body.\");\n"
"\t\t__builtin_puts(\"Filename:\");\n"
"\t\t__builtin_puts(following.filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, following.linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, following.colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\n"
"\tfor(walker = following, 1, walker = walker.right)\n"
"\t\t//we definitely won't find anything if we're null, or our right\n"
"\t\tif( walker == 0) break end\n"
"\t\tif(walker.right == 0) break end\n"
"\t\t//we definitely won't find anything if our text or our right's text is null.\n"
"\t\tif( walker.text == 0) continue end\n"
"\t\tif( walker.right.text == 0) continue end\n"
"\t\tif(\n"
"\t\t\t(walker.d == CG_TOK_OPERATOR) && (walker.text streq \"@\")\n"
"\t\t)\n"
"\t\t\t//check if we have one!\n"
"\t\t\tcgstr* m\n"
"\t\t\tcgstr f\n"
"\t\t\tf.s = walker.right.text;\n"
"\t\t\tm = builder_hook_namespace_query(f);\n"
"\t\t\tif(m != 0)\n"
"\t\t\t\t//do the replacement.\n"
"\t\t\t\tf:free();\n"
"\t\t\t\twalker.right.text = __builtin_strdup(m.s);\n"
"\t\t\tend\n"
"\t\tend\n"
"\tend\n"
"\t\n"
"\n"
"\tretval =  (cg_qtok(\"@\"));\n"
"\tretval:push_end(cg_qtok(\"bldr\"));\n"
"\tretval:push_end(inlist_dupe);\n"
"\tretval:push_end(cg_qtok(\"[\"));\n"
"\tretval:push_end(following);\n"
"\tretval:push_end(cg_qtok(\"]\"));\n"
"\treturn retval;\n"
"]\n"
"\n"
"//TODO: mkbldrn\n"
"\n"
"\n"
"//mkbldrn\n"
"//actually somewhat simpler than mkbldr\n"
"//it takes a number of arguments expected\n"
"@bldrn mkbldrn 3 [\n"
"\tcgstrll* retval\n"
"\tcgstrll* inlist_dupe\n"
"\tcgstrll* following\n"
"\tcgstrll* walker\n"
"\t\n"
"\tcgstr name\n"
"\t\n"
"\n"
"\tinlist_dupe = inlist:dupell();\n"
"\tfollowing = inlist_dupe.right;\n"
"\n"
"\t//@cg_bldr_ifnull [following]\n"
"\tif(following == 0)\n"
"\t\tchar[500] buf;\n"
"\t\t__builtin_puts(\"MKBUILDER_ERROR\");\n"
"\t\t__builtin_puts(\"Invoked mkbldrn with null where the number of args should be\");\n"
"\t\t__builtin_puts(\"Filename:\");\n"
"\t\t__builtin_puts(following.filename);\n"
"\t\t__builtin_puts(\"Linenum:\");\n"
"\t\t__builtin_utoa(buf, following.linenum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_puts(\"Colnum:\");\n"
"\t\t__builtin_utoa(buf, following.colnum);\n"
"\t\t__builtin_puts(buf);\n"
"\t\t__builtin_exit(1);\n"
"\tend\n"
"\n"
"\tfor(walker = following, 1, walker = walker.right)\n"
"\t\t//we definitely won't find anything if we're null, or our right\n"
"\t\t//@cg_bldr_ifnull [walker] break end\n"
"\t\t//@cg_bldr_ifnull [walker.right] break end\n"
"\t\tif(walker == 0) break end\n"
"\t\tif(walker.right == 0) break end\n"
"\t\t//we definitely won't find anything if our text or our right's text is null.\n"
"\t\t//@cg_bldr_ifnull [walker.text] continue end\n"
"\t\t//@cg_bldr_ifnull [walker.right.text] continue end\n"
"\t\tif(walker.text == 0) continue end\n"
"\t\tif(walker.right.text == 0) continue end\n"
"\t\tif(\n"
"\t\t\t(walker.d == CG_TOK_OPERATOR) && (walker.text streq \"@\")\n"
"\t\t)\n"
"\t\t\t//check if we have one!\n"
"\t\t\tcgstr* m\n"
"\t\t\tcgstr f\n"
"\t\t\tf.s = walker.right.text;\n"
"\t\t\tm = builder_hook_namespace_query(f);\n"
"\t\t\t//@cg_bldr_ifnotnull [m]\n"
"\t\t\tif(m != 0)\n"
"\t\t\t\t//do the replacement.\n"
"\t\t\t\tf:free();\n"
"\t\t\t\twalker.right.text = __builtin_strdup(m.s);\n"
"\t\t\tend\n"
"\t\tend\n"
"\tend\n"
"\t\n"
"\n"
"\tretval =  (cg_qtok(\"@\")); walker = retval;\n"
"\tretval:push_end(cg_qtok(\"bldrn\")); walker = walker.right;\n"
"\tretval:push_end(inlist_dupe);\n"
"\treturn retval;\n"
"]\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"\n"
"@bldrn cg_bldr_dupell_inlist 0 [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t\t\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\tcgstrll* retval\n"
"\tcgstrll* inlist_dupe\n"
"\tcgstrll* rwalk\n"
"\n"
"\tretval = cg_qtok(\"cgstrll\");rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"inlist_dupe\"));rwalk = rwalk.right;\n"
"\n"
"\trwalk:push_end(cg_qtok(\"inlist_dupe\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"dupell\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\")  );rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\";\"));\n"
"\treturn retval;\n"
"]\n"
"\n"
"\t\t\n"
"@bldrn cg_bldr_dupe_inlist 0 [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\tcgstrll* retval\n"
"\tcgstrll* inlist_dupe\n"
"\tcgstrll* rwalk\n"
"\n"
"\n"
"\tretval = cg_qtok(\"cgstrll\");rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"*\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"inlist_dupe\"));rwalk = rwalk.right;\n"
"\n"
"\trwalk:push_end(cg_qtok(\"inlist_dupe\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"inlist\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"dupe\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\")  );rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\";\"));\n"
"\treturn retval;\n"
"]\n"
"\n"
"//Tools for making builders..\n"
"//TOOL 1: THE QTOK PUSHER\n"
"//one of the most often used functions in the entire mp library...\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_pushqtok\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\tcgstrll* retval\n"
"\tcgstrll* rwalk\n"
"\t\n"
"\tretval =       cg_qtok(\"rwalk\");rwalk = retval;\n"
"\t//rwalk:push_end(cg_qtok(\":\"));rwalk = rwalk.right;\n"
"\t//rwalk:push_end(cg_qtok(\"push_end\"));rwalk = rwalk.right;\n"
"    rwalk:push_end(cg_qtok(\".\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"right\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\t\n"
"\t//rwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"\t\n"
"\trwalk:push_end(cg_qtok(\"cg_qtok\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"(\"));rwalk = rwalk.right;\n"
"    rwalk:push_end(inlist:dupell());rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\t\n"
"\t//rwalk:push_end(cg_qtok(\")\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t\n"
"\trwalk:push_end(cg_qtok(\"rwalk\"));rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"=\"));rwalk = rwalk.right;\n"
"\trwalk:push_end(cg_qtok(\"rwalk\"));rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\".\"));rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\"right\"));rwalk = retval;\n"
"\trwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"\t//retval:debug_print();\n"
"\treturn retval;\n"
"]\n"
"\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_initqtok\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t@dupell_inlist ;\n"
"\t\n"
"\t//automate this line.\n"
"\tcgstrll* retval\n"
"\tcgstrll* rwalk\n"
"\tretval = cg_qtok(\"cgstrll\");rwalk = retval;\n"
"\t\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"retval\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"cg_qtok\"\n"
"\t@pushqtok \"(\"\n"
"\trwalk:push_end(inlist_dupe);\n"
"    while(rwalk.right != 0)\n"
"        rwalk = rwalk.right\n"
"    end\n"
"\t@pushqtok \")\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \";\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"//same as above, but no variable declarations.\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_initqtok_nodecl\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t\n"
"\t@initqtok \"retval\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"cg_qtok\"\n"
"\t@pushqtok \"(\"\n"
"    rwalk:push_end(inlist:dupe());\n"
"    while(rwalk.right != 0)\n"
"        rwalk = rwalk.right\n"
"    end\n"
"\t@pushqtok \")\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \";\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_initvars\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t@initqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"retval\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"rwalk\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_initvars_dummy\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t@initqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"retval\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"@\"\n"
"\t@pushqtok \"cg_bldr_initqtok\"\n"
"\t@pushqtok \"\\\"#\\\"\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_initvars_dummy_var\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t@initqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"retval\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"dummy\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t\n"
"\t@pushqtok \"dummy\"\n"
"\t@pushqtok \".\"\n"
"\t@pushqtok \"text\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"\\\"#\\\"\"\n"
"\t@pushqtok \";\"\n"
"    @pushqtok \"dummy\"\n"
"\t@pushqtok \".\"\n"
"    @pushqtok \"d\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"\\\"0\\\"\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"dummy\"\n"
"\t@pushqtok \";\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_inittok\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"    @dupell_inlist ;\n"
"\t@initqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"retval\"\n"
"\t\n"
"\t@pushqtok \"cgstrll\"\n"
"\t@pushqtok \"*\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \"=\"\n"
"\t//not qtok!\n"
"\t//@pushqtok \"cg_qtok\"\n"
"\t@pushqtok \"(\"\n"
"    rwalk.right = (inlist_dupe);\n"
"    while(rwalk.right != 0)\n"
"        rwalk = rwalk.right\n"
"    end\n"
"\t@pushqtok \")\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \";\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_inittok_nodecl\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"    @dupell_inlist ;\n"
"\t\n"
"\t@initqtok \"retval\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"cg_qtok\"\n"
"\t@pushqtok \"(\"\n"
"    rwalk:push_end(inlist_dupe);\n"
"    while(rwalk.right != 0)\n"
"        rwalk = rwalk.right\n"
"    end\n"
"\t@pushqtok \")\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"retval\"\n"
"\t@pushqtok \";\"\n"
"\treturn retval;\n"
"]\n"
"\n"
"\n"
"//rwalk:push_end(cg_qtok(\";\"));rwalk = rwalk.right;\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\t    cg_bldr_pushtok\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"\n"
"\t@initqtok \"rwalk\"\n"
"\t@pushqtok \".\"\n"
"\t@pushqtok \"right\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"(\"\n"
"\t    rwalk:push_end(inlist:dupell());\n"
"\t    while(rwalk.right != 0)\n"
"\t        rwalk = rwalk.right\n"
"\t    end\n"
"\t@pushqtok \")\"\n"
"\t@pushqtok \";\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \"=\"\n"
"\t@pushqtok \"rwalk\"\n"
"\t@pushqtok \".\"\n"
"\t@pushqtok \"right\"\n"
"\t@pushqtok \";\"\n"
"\t\n"
"\treturn retval;\n"
"]\n"
"\n"
"@mkbldr [\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"    cg_bldr_rwalk_skip\n"
"\t//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n"
"    @initqtok \"while\"\n"
"    @pushqtok \"(\"\n"
"        @pushqtok \"rwalk\"\n"
"        @pushqtok \".\"\n"
"        @pushqtok \"right\"\n"
"        @pushqtok \"!=\"\n"
"        @pushqtok \"cast\"\n"
"        @pushqtok \"(\"\n"
"            @pushqtok \"cgstrll\"\n"
"            @pushqtok \"*\"\n"
"        @pushqtok \")\"\n"
"        @pushqtok \"0\"\n"
"    @pushqtok \")\"\n"
"        @pushqtok \"rwalk\"\n"
"        @pushqtok \"=\"\n"
"        @pushqtok \"rwalk\"\n"
"        @pushqtok \".\"\n"
"        @pushqtok \"right\"\n"
"        @pushqtok \";\"\n"
"    @pushqtok \"end\"\n"
"    return retval;\n"
"]\n"
"\n"
""
