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
static char** functions = NULL;
static int functions_ptr = 0;
static int counter = 0;
static int arguments_ptr = 0;
static char* arguments[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static const char* build_dir = "build/";
static const char* src_dir = "src/";
static const int DEFAULT_SIZE = 32;
static const int align = 32;

void assign_variable(){
    fprintf(write_ptr,
        "pop rax \n"
        "pop rbx \n"
        "mov [rbx], rax \n\n");
}

void mmap(int size){
    fprintf(write_ptr,
        "mov rax, 9 \n"
        "mov rsi, %d \n"
        "mov rdi, 0 \n"
        "mov rdx, 3 \n"
        "mov r10, 34 \n"
        "mov r8, -1 \n"
        "mov r9, 0 \n"
        "syscall \n"
        "push rax \n", size * align);
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
        "jne block%d \n\n", idem_key, idem_key);
}

void cond_while(int idem_key){
    fprintf(write_ptr, 
        "pop rax \n"
        "cmp rax, 1 \n"
        "je cond_while%d \n"
        "jne block%d \n\n", idem_key, idem_key);
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
    free(arr);
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
        (text_ptr == 1 && strchr(";\n ([{}])", text[0]) != NULL)) return 1; // bracket or newline
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
        fprintf(write_ptr, "push %s \n", text);
    }
}

void set_variable(char* text){
    fprintf(write_ptr, "; set %s\n", text);
    fprintf(write_ptr,
        "pop rax \n"
        "mov [rbp-%d], rax \n\n", vars_global_ptr * align + align);
    vars_global[vars_global_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(vars_global[vars_global_ptr++], text);
}

/*
void set_function_parameter_variable(char* text){
    fprintf(write_ptr, "; set func var |%s|\n", text);
    fprintf(write_ptr, "mov [rbp-%d], %s \n\n", vars_local_ptr * align + align, arguments[vars_local_ptr]);
    vars_local[vars_local_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(vars_local[vars_local_ptr++], text);
}
*/

void recall_variable_primitive(char* text){
    if(strcmp(text, "") != 0 && is_letter_str(text)){
        fprintf(write_ptr, "; recall %s\n", text);
        int offset = find_in_array(text, vars_global, vars_global_ptr);
        if(offset < 0) return;
        fprintf(write_ptr,
            "mov rax, rbp \n"
            "sub rax, %d \n"
            "mov rbx, [rax] \n"
            "push rbx \n\n", offset * align + align);
    }
}

void recall_variable_array(char* text){
    if(strcmp(text, "") != 0 && is_letter_str(text)){
        fprintf(write_ptr, "; recall %s\n", text);
        int offset = find_in_array(text, vars_global, vars_global_ptr);
        if(offset < 0) return;
        fprintf(write_ptr,
            "mov rbx, [rbp-%d] \n"
            "pop rax \n"
            "imul rax, %d \n"
            "add rbx, rax \n"
            "mov rax, [rbx] \n"
            "push rax \n\n", offset * align + align, align);
    }
}

void update_variable_primitive(char* text){
    fprintf(write_ptr, "; update %s\n", text);
    int offset = find_in_array(text, vars_global, vars_global_ptr);
    if(offset < 0) return;
    fprintf(write_ptr, 
        "mov rax, rbp \n"
        "sub rax, %d \n"
        "push rax \n\n", offset * align + align);
}

void update_variable_array(char* text){
    fprintf(write_ptr, "; update %s\n", text);
    int offset = find_in_array(text, vars_global, vars_global_ptr);
    if(offset < 0) return;
    fprintf(write_ptr,
        "mov rbx, [rbp-%d] \n"
        "pop rax \n"
        "imul rax, %d \n"
        "add rbx, rax \n"
        "push rbx \n", offset * align + align, align);
}

void recall_or_update_variable_array(char* text){
    char* left_brackets = read_token();
    if(strcmp(left_brackets, "[") == 0){ 
        ungetstring(left_brackets);
        check_next_word(" ");
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
    ungetstring(left_brackets);
    free(left_brackets);
}

void recall_or_update_variable_primitive(char* text){
    char* eq = read_token();
    if(strcmp(eq, "=") == 0){        // update
        update_variable_primitive(text);
        read_until_token(";");
        assign_variable();
    }
    else{                            // recall
        ungetstring(eq);
        recall_variable_primitive(text);
    }
    free(eq);
}

void recall_or_update_variable(char* text){
    if(find_in_array(text, vars_global, vars_global_ptr) >= 0){
        recall_or_update_variable_array(text);
        recall_or_update_variable_primitive(text);
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
        "block%d: \n", idem_key, idem_key);
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
        "block%d: \n", idem_key, idem_key);
}

void handle_conditional(char* text, int idem_key){
    if(strcmp(text, "if") == 0) handle_conditional_if(idem_key);
    else if(strcmp(text, "while") == 0) handle_conditional_while(idem_key);
}

void handle_array_declaration(char* var_name){
    char* size = read_token();
    check_next_word("]");
    mmap(atoi(size));
    set_variable(var_name);
    free(size);
}

void handle_primitive_declaration(char* match, char* var_name){
    read_until_token(match);
    set_variable(var_name);
}

void handle_function_declaration(){
    /*
    else if(strcmp(next_token, "(") == 0){    // function
        fprintf(write_ptr, "\nfunc %d: \n", idem_key);
        free(read_chars(")"));
        check_next_word("{");
        free(read_chars("}"));
        fprintf(write_ptr, "ret \n");
    }
    */
}

void handle_declaration(char* text, char* match, int idem_key){
    check_next_word(" ");
    char* var_name = read_token();
    char* next_token = read_token();
    if(strcmp(next_token, "[") == 0) handle_array_declaration(var_name);
    else if(strcmp(next_token, "=") == 0) handle_primitive_declaration(match, var_name);
    handle_function_declaration();
    free(next_token);
    free(var_name);
}

void handle_builtin_function(char* text, char* match, int idem_key){
    if(strcmp(text, "print") == 0){
        check_next_word("(");
        read_until_token(")");
        print_int(write_ptr);
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
        char* token = read_token();
        if(handle_operator(token, match) // pass control flow
            || strcmp(match, token) == 0){  // match found
            free(token);
            return;
        }
        if(strcmp(token, "int") == 0) handle_declaration(token, match, counter++);
        handle_builtin_function(token, match, counter++);
        handle_conditional(token, counter++);
        match_opposite_delimiter(token);
        free(token);
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
            fprintf(write_ptr, "; token: |%s|\n", text);
            store_number(text);
            recall_or_update_variable(text);
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

    read_ptr = fopen(read_path, "r");
    write_ptr = fopen(write_path, "w");
    vars_global = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);
    vars_local = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);
    functions = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);

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
    free_array(functions, functions_ptr);

    pclose(read_ptr);
    fclose(write_ptr);
}

int main(int argc, char* argv[]){
    compile(argv[1]);
    return 0;
}

