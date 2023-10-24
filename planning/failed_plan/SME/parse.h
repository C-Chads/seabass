


//TODO: write lexer
strll* peek(){
    return NULL; //TODO
}

void consume(){
    return; //TODO
}

//Rough outline of the parser...


void parseUnit(
    sme_ast_mod_root** root
){


}

void parseType(
    sme_type* node
){

}

//parse a typedef.
void parseTypeDecl(
    sme_type* node
){

}

void parseGvar(
    sme_ast_vardecl* node
){
    
}


i64 parseIntConstexpr(){

}
i64 parseFloatConstexpr(){

}

void parseFn(sme_ast_fn* node){
    //todo
}

void parseMethod(sme_ast_fn* node){

}

void parseScope(sme_ast_scope* scope){

}

void parseInsn(sme_ast_insn* node){

}

void parseCombonode(sme_ast_combo_node* node){
    //parse into combo node.
}

