#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <compile.h>

static FILE* read_ptr = NULL;
static FILE* write_ptr = NULL; 
static char** vars_global = NULL;
static int vars_global_ptr = 0;
static char** vars_local = NULL;
static int vars_local_ptr = 0;
static char** function_names = NULL;
static int function_names_ptr = 0;
static char** function_blocks = NULL;
static int function_blocks_ptr = 0;
static int counter = 0;
static int scope = 0; // 0 = global ; 1 = local ; 2 = invoking function ; 3 = defining function
static char*** vars_cur = NULL;
static int* vars_cur_ptr = 0;
static int arguments_ptr = 0;
static char* arguments[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static const char* build_dir = "build/";
static const char* src_dir = "src/";
static const int DEFAULT_SIZE = 64;
static const int align = 64;

void assign_variable(){
    fprintf(write_ptr,
        "pop rax \n"
        "pop rbx \n"
        "mov [rbx], rax \n\n");
}

void mmap(){
    fprintf(write_ptr,
        "mov rax, 9 \n"
        "pop rsi \n"
        "imul rsi, %d \n"
        "mov rdi, 0 \n"
        "mov rdx, 3 \n"
        "mov r10, 34 \n"
        "mov r8, -1 \n"
        "mov r9, 0 \n"
        "syscall \n"
        "push rax \n", 
        align);
}

void print_int(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "lea rdi, [rbx] \n"
        "call print_int \n\n");
}

void cond_if(int idem_key){
    fprintf(write_ptr, 
        "pop rax \n"
        "cmp rax, 1 \n"
        "je cond_if%d \n"
        "jne block%d \n\n", 
        idem_key, idem_key);
}

void cond_while(int idem_key){
    fprintf(write_ptr, 
        "pop rax \n"
        "cmp rax, 1 \n"
        "je cond_while%d \n"
        "jne block%d \n\n", 
        idem_key, idem_key);
}

void or(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "or rax, rbx \n"
        "push rax \n\n");
}

void and(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "and rax, rbx \n"
        "push rax \n\n");
}

void greater(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "cmp rax, rbx \n"
        "setg al \n"
        "push rax \n\n");
}

void less(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "cmp rax, rbx \n"
        "setl al \n"
        "push rax \n\n");
}

void unequal(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "xor al, al"
        "setnz al \n"
        "push rax \n\n");
}

void equal(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setz al \n"
        "push rax \n\n");
}

void add(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "add rax, rbx \n"
        "push rax \n\n");
}

void sub(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "sub rax, rbx \n"
        "push rax \n\n");
}

void mul(){
    fprintf(write_ptr,
        "pop rax \n"
        "pop rbx \n"
        "imul rax, rbx \n"
        "push rax \n\n");
}

void idiv(){
    fprintf(write_ptr,
        "pop rbx \n"
        "pop rax \n"
        "idiv rbx \n"
        "push rax \n\n");
}

// return assumes_control_flow
int handle_operator(char* text, char* match){
    if(strcmp(text, "-") == 0){
        read_until_token(match);
        sub();
        return 1;
    }
    else if(strcmp(text, "+") == 0){
        read_until_token(match);
        add();
        return 1;
    }
    else if(strcmp(text, "*") == 0){
        char* next = read_token();
        match_opposite_delimiter(next);
        free(next);
        mul();
        return 0;
    }
    else if(strcmp(text, "/") == 0){
        char* next = read_token();
        match_opposite_delimiter(next);
        free(next);
        idiv();
        return 0;
    }
    else if(strcmp(text, ">") == 0){
        free(read_token());
        greater();
        return 0;
    }
    else if(strcmp(text, "<") == 0){
        free(read_token());
        less();
        return 0;
    }
    else if(strcmp(text, "==") == 0){
        free(read_token());
        equal();
        return 0;
    }
    else if(strcmp(text, "!=") == 0){
        free(read_token());
        equal();
        return 0;
    }
    else if(strcmp(text, "||") == 0){
        read_until_token(match);
        or();
        return 1;
    }
    else if(strcmp(text, "&&") == 0){
        read_until_token(match);
        and();
        return 1;
    }
    else return 0;
}

void append_array_char(char** arr, int* arr_ptr, char* text){
    arr[(*arr_ptr)] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(arr[(*arr_ptr)++], text);
}

int find_in_array(char* text, char** arr, int arr_ptr){
    int offset = -1;
    for(int i = 0; i < arr_ptr; ++i){
        if(strcmp(arr[i], text) == 0) offset = i;
    }
    return offset;
}

void free_array(char** arr, int arr_ptr){
    for(int i = 0; i < arr_ptr; ++i){
        free(arr[i]);
    }
}

int is_num_char(char in){
    return strchr("0123456789", in) != NULL;
}

int is_num_str(char* text){
    for(int i = 0; text[i] != '\0'; ++i){
        if(!is_num_char(text[i])){
            return 0;
        }
    }
    return 1;
}

int is_operator(char in){
    return strchr("/*-+=&|<>", in) != NULL;
}

int is_letter_char(char in){
    return strchr("abcdefghijklmnopqrstuvwxyz", in) != NULL;
}

int is_letter_str(char* text){
    for(int i = 0; text[i] != '\0'; ++i){
        if(!is_letter_char(text[i])){
            return 0;
        }
    }
    return 1;

}

int get_type(char in){
    if(is_num_char(in)) return 0;
    else if(is_operator(in)) return 1;
    else if(is_letter_char(in)) return 2;
    else return 3;
}

int check_should_terminate(char* text, int text_ptr, char cur){
    if((text_ptr > 0 && get_type(text[text_ptr-1]) != get_type(cur)) || // changing type
        (text_ptr == 1 && strchr(",;\n ([{}])", text[0]) != NULL)) return 1; // bracket or newline
    else return 0;
}

void ungetstring(char* text){
    for(int i = 0; i < strlen(text); ++i){
        ungetc(text[i], read_ptr);
    }
}

void match_opposite_delimiter(char* text){
    if(strcmp(text, "(") == 0) read_until_token(")");
    else if(strcmp(text, "{") == 0) read_until_token("}");
    else if(strcmp(text, "[") == 0) read_until_token("]");
}

void store_number(char* text){
    if(is_num_str(text)){
        if(scope == 0) fprintf(write_ptr, "push %s \n", text);
        else if(scope == 2) fprintf(write_ptr, "mov %s, %s \n", arguments[arguments_ptr++], text);
    }
}

void declare_variable(char* text){
    char* init = NULL;
    if(scope == 0) init = "0";
    else if(scope == 3) init = arguments[arguments_ptr++];
    fprintf(write_ptr,
        "; declare %s\n"
        "mov qword [rbp-%d], %s \n\n", 
        text, (*vars_cur_ptr) * align + align, init);
    append_array_char(*vars_cur, vars_cur_ptr, text);
}

void construct_variable(char* text){
    fprintf(write_ptr,
        "; construct %s\n"
        "pop rax \n"
        "mov qword [rbp-%d], rax \n\n", 
        text, (*vars_cur_ptr) * align + align);
    append_array_char(*vars_cur, vars_cur_ptr, text);
}

void recall_variable_primitive(char* text){
    if(strcmp(text, "") != 0 && is_letter_str(text)){
        fprintf(write_ptr, "; seeking %s\n", text);
        int offset = find_in_array(text, *vars_cur, *vars_cur_ptr);
        if(offset < 0) return;
        fprintf(write_ptr,
            "; recall %s \n"
            "mov rax, rbp \n"
            "sub rax, %d \n"
            "mov rbx, [rax] \n"
            "push rbx \n\n", 
            text, offset * align + align);
    }
}

void recall_variable_array(char* text){
    if(strcmp(text, "") != 0 && is_letter_str(text)){
        int offset = find_in_array(text, vars_global, vars_global_ptr);
        if(offset < 0) return;
        fprintf(write_ptr,
            "; recall %s \n"
            "mov rbx, [rbp-%d] \n"
            "pop rax \n"
            "imul rax, %d \n"
            "add rbx, rax \n"
            "mov rax, [rbx] \n"
            "push rax \n\n", 
            text, offset * align + align, align);
    }
}

void update_variable_primitive(char* text){
    int offset = find_in_array(text, vars_global, vars_global_ptr);
    if(offset < 0) return;
    fprintf(write_ptr, 
        "; update %s \n"
        "mov rax, rbp \n"
        "sub rax, %d \n"
        "push rax \n\n", 
        text, offset * align + align);
}

void update_variable_array(char* text){
    int offset = find_in_array(text, vars_global, vars_global_ptr);
    if(offset < 0) return;
    fprintf(write_ptr,
        "; update %s \n"
        "mov rbx, [rbp-%d] \n"
        "pop rax \n"
        "imul rax, %d \n"
        "add rbx, rax \n"
        "push rbx \n", 
        text, offset * align + align, align);
}

int recall_or_update_resource_array(char* text){
    free(read_token());
    check_next_word("]");
    char* eq = read_token();
    if(strcmp(eq, "=") == 0){        // update
        update_variable_array(text);
        read_until_token(";");
        assign_variable();
    }
    else{                            // recall
        ungetstring(eq);
        recall_variable_array(text);
    }
    free(eq);
}

void recall_or_update_resource(char* text){
    int function_pos = find_in_array(text, function_names, function_names_ptr);
    if(find_in_array(text, *vars_cur, *vars_cur_ptr) >= 0){ // variable
        char* next_token = read_token();
        if(strcmp(next_token, "[") == 0){ // array
            recall_or_update_resource_array(text);
        }
        else if(strcmp(next_token, "=") == 0){ // update primitive
            update_variable_primitive(text);
            read_until_token(";");
            assign_variable();
        }
        else{ // recall primitive
            ungetstring(next_token);
            recall_variable_primitive(text);
        }
        free(next_token);
    }
    else if(function_pos >= 0){ // invoke function
        check_next_word("(");
        scope = 2;
        read_until_token(")");
        scope = 0;
        fprintf(write_ptr, 
            "call %s \n" 
            "push rax \n", 
            function_blocks[function_pos]);
    }
}

void handle_conditional_if(int idem_key){
    check_next_word("(");
    read_until_token(")");
    fprintf(write_ptr, "; found ) \n");
    cond_if(idem_key);
    fprintf(write_ptr, "cond_if%d: \n", idem_key);
    check_next_word("{");
    read_until_token("}");
    fprintf(write_ptr, 
        "jmp block%d \n"
        "block%d: \n", 
        idem_key, idem_key);
}

void handle_conditional_while(int idem_key){
    fprintf(write_ptr, 
        "jmp pre_while%d \n"
        "pre_while%d: \n", idem_key, idem_key);
    check_next_word("(");
    read_until_token(")");
    cond_while(idem_key);
    fprintf(write_ptr, "cond_while%d: \n", idem_key);
    check_next_word("{");
    read_until_token("}");
    fprintf(write_ptr, 
        "jmp pre_while%d \n"
        "block%d: \n", 
        idem_key, idem_key);
}

void handle_conditional(char* text, int idem_key){
    if(strcmp(text, "if") == 0) handle_conditional_if(idem_key);
    else if(strcmp(text, "while") == 0) handle_conditional_while(idem_key);
}

void handle_function_declaration(int idem_key){
    vars_cur = &vars_local;
    vars_cur_ptr = &vars_local_ptr;
    fprintf(write_ptr, 
        "jmp block_%d \n"
        "func_%d: \n", 
        idem_key, idem_key);
    scope = 3;
    read_until_token(")");
    check_next_word("{");
    scope = 1;
    read_until_token("}");
    scope = 0;
    arguments_ptr = 0;
    vars_local_ptr = 0;
    vars_cur = &vars_global;
    vars_cur_ptr = &vars_global_ptr;
    free_array(vars_local, vars_local_ptr);
    fprintf(write_ptr, 
        "ret \n"
        "block_%d:\n", 
        idem_key);
}

void handle_declaration(char* text, char* match, int idem_key){
    if(strcmp(text, "int") == 0){
        check_next_word(" ");
    }
    else if(strcmp(text, "static") == 0){
        check_next_word(" ");
        check_next_word("int");
    }
    else if(strcmp(text, "void") == 0){
        check_next_word(" ");
    }
    else return;
    char* var_name = read_token();
    char* next_token = read_token();
    if(strcmp(next_token, "[") == 0){ // array
        free(read_token());
        check_next_word("]");
        mmap();
        construct_variable(var_name);
    }
    else if(strcmp(next_token, "=") == 0){ // construct primitive
        read_until_token(match);
        construct_variable(var_name);
    }
    else if(strcmp(next_token, ",") == 0 || strcmp(next_token, ";") == 0 || strcmp(next_token, ")") == 0){ // declare primitive
        ungetstring(next_token);
        declare_variable(var_name);
    }
    else if(strcmp(next_token, "(") == 0){ // function
        fprintf(write_ptr, "; declare function %s \n", var_name);
        char* block_buffer = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
        sprintf(block_buffer, "func_%d", counter);
        append_array_char(function_names, &function_names_ptr, var_name);
        append_array_char(function_blocks, &function_blocks_ptr, block_buffer); // this will be freed at the end of the file
        handle_function_declaration(counter);
    }
    free(next_token);
    free(var_name);
}

void handle_builtin_function(char* text, char* match, int idem_key){
    if(strcmp(text, "print") == 0){
        check_next_word("(");
        read_until_token(")");
        print_int(write_ptr);
    }
    else if(strcmp(text, "return") == 0){
        read_until_token(";");
        fprintf(write_ptr, 
            "pop rax \n"
            "ret \n"
            "block_%d:\n", 
            idem_key);
    }
}

void check_next_word(char* text){
    char* next = read_token();
    if(strcmp(next, text) != 0){
        free(next);
        fprintf(write_ptr, "; Expected %s found %s\n", text, next);
        exit(0);
    }
    free(next);
}

void read_until_token(char* match){
    fprintf(write_ptr, "; Looking for %s\n", match);
    while(feof(read_ptr) == 0){
        char* next_token = read_token();
        if(handle_operator(next_token, match) // pass control flow
            || strcmp(match, next_token) == 0){  // match found
            fprintf(write_ptr, "; Found %s\n", match);
            free(next_token);
            return;
        }
        handle_declaration(next_token, match, counter++);
        handle_builtin_function(next_token, match, counter++);
        handle_conditional(next_token, counter++);
        match_opposite_delimiter(next_token);
        free(next_token);
    }
}

char* read_token(){
    char *text = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    text[0] = '\0';
    int text_ptr = 0;
    while(feof(read_ptr) == 0){
        int cur = fgetc(read_ptr);
        if(check_should_terminate(text, text_ptr, cur)){
            ungetc(cur, read_ptr);
            store_number(text);
            recall_or_update_resource(text);
            return text;
        }
        else{
            text[text_ptr++] = cur;
            text[text_ptr] = '\0';
        }
    }
    return text;
}

void compile(char *input_file){
    char read_path[DEFAULT_SIZE];
    char write_path[DEFAULT_SIZE];
    strcpy(read_path, src_dir);
    strcpy(write_path, build_dir);
    strcat(read_path, input_file);
    strcat(write_path, "main.asm");
    vars_cur = &vars_global;
    vars_cur_ptr = &vars_global_ptr;

    read_ptr = fopen(read_path, "r");
    write_ptr = fopen(write_path, "w");
    vars_global = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);
    vars_local = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);
    function_names = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);
    function_blocks = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);

    fprintf(write_ptr,
        "global _start \n" 
        "extern print_int \n" 
        "extern int_to_string \n" 
        "extern exit \n \n" 
        "section .text \n" 
        "_start: \n"
        "mov rbp, rsp \n"); 

    while(feof(read_ptr) == 0){
        read_until_token(";");
    }

    fprintf(write_ptr, "call exit\n");

    free_array(vars_global, vars_global_ptr);
    free_array(vars_local, vars_local_ptr);
    free_array(function_names, function_names_ptr);
    free_array(function_blocks, function_blocks_ptr);
    free(vars_global);
    free(vars_local);
    free(function_names);
    free(function_blocks);

    pclose(read_ptr);
    fclose(write_ptr);
}

int main(int argc, char* argv[]){
    compile(argv[1]);
    return 0;
}

