#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <compile.h>
#define is_changed (text_ptr > 0 && get_type(text[text_ptr-1]) != get_type(cur))
#define is_terminated (length > 0 && --length == 0)

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

int handler_math_or_bool_operator(char* text, int* text_ptr, char* match){
    if(strcmp(text, "-") == 0){
        free(read_chars(0, match, 0));
        sub();
        return 1;
    }
    else if(strcmp(text, "+") == 0){
        free(read_chars(0, match, 0));
        add();
        return 1;
    }
    else if(strcmp(text, "*") == 0){
        free(read_chars(1, match, 0));
        mul();
        return 0;
    }
    else if(strcmp(text, "/") == 0){
        free(read_chars(1, match, 0));
        idiv();
        return 0;
    }
    else if(strcmp(text, ">") == 0){
        free(read_chars(1, match, 0));
        greater();
        return 0;
    }
    else if(strcmp(text, "<") == 0){
        free(read_chars(1, match, 0));
        less();
        return 0;
    }
    else if(strcmp(text, "==") == 0){
        free(read_chars(1, match, 0));
        equal();
        return 0;
    }
    else if(strcmp(text, "!=") == 0){
        free(read_chars(1, match, 0));
        equal();
        return 0;
    }
    else if(strcmp(text, "||") == 0){
        free(read_chars(0, match, 0));
        or();
        return 1;
    }
    else if(strcmp(text, "&&") == 0){
        free(read_chars(0, match, 0));
        and();
        return 1;
    }
    else return 0;
}

int is_num(char in){
    return strchr("0123456789", in) != NULL;
}

int is_operator(char in){
    return strchr("/*-+=&|<>", in) != NULL;
}

int is_letter(char in){
    return strchr("abcdefghijklmnopqrstuvwxyz", in) != NULL;
}

int get_type(char in){
    if(is_num(in)) return 0;
    else if(is_operator(in)) return 1;
    else if(is_letter(in)) return 2;
    else if(in == ';') return 3;
    else if(in == '\n') return 4;
    else if(in == ' ') return 5;
    else if(in == ',') return 6;
    else if(in == ')') return 7;
    else if(in == '}') return 8;
    else if(in == ']') return 9;
    else return 10;
}

void ungetstring(char* text){
    for(int i = 0; i < strlen(text); ++i){
        ungetc(text[i], read_ptr);
    }
}

void match_char(char* text){
    if(strcmp(text, "(") == 0) free(read_chars(0, ")", 0));
    else if(strcmp(text, "{") == 0) free(read_chars(0, "}", 0));
    else if(strcmp(text, "[") == 0) free(read_chars(0, "]", 0));
}

int store_number(char* num, int* num_ptr){
    if(*num_ptr > 0 && is_num(num[*num_ptr-1])){
        fprintf(write_ptr, "push %s \n", num);
        *num_ptr = 0;
        num[0] = '\0';
        return 1;
    }
    return 0;
}

void set_variable(char* text){
    fprintf(write_ptr, "; set |%s|\n", text);
    fprintf(write_ptr,
        "pop rax \n"
        "mov [rbp-%d], rax \n\n", vars_global_ptr * align + align);
    vars_global[vars_global_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(vars_global[vars_global_ptr++], text);
}

void set_function_parameter_variable(char* text){
    fprintf(write_ptr, "; set func var |%s|\n", text);
    fprintf(write_ptr, "mov [rbp-%d], %s \n\n", vars_local_ptr * align + align, arguments[vars_local_ptr]);
    vars_local[vars_local_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(vars_local[vars_local_ptr++], text);
}

void recall_variable(char* text, int* text_ptr, int index){
    if(*text_ptr > 0 && is_letter(text[*text_ptr-1])){
        fprintf(write_ptr, "; recall |%s|\n", text);
        int offset = find_in_array(text, vars_global, vars_global_ptr);
        if(offset < 0) return;
        if(index > -1){ // array
            fprintf(write_ptr,
                "mov rbx, [rbp-%d] \n"
                "pop rax \n"
                "imul rax, %d \n"
                "add rbx, rax \n"
                "mov rax, [rbx] \n"
                "push rax \n\n", offset * align + align, align);
        }
        else{           // primitive
            fprintf(write_ptr,
                "mov rax, rbp \n"
                "sub rax, %d \n"
                "mov rbx, [rax] \n"
                "push rbx \n\n", offset * align + align);
        }
        *text_ptr = 0;
        text[0] = '\0';
    }
}

void update_variable(char* text, int index){
    fprintf(write_ptr, "; update |%s|\n", text);
    int offset = find_in_array(text, vars_global, vars_global_ptr);
    if(offset < 0) return;
    if(index > -1){ // array
        fprintf(write_ptr,
            "mov rbx, [rbp-%d] \n"
            "pop rax \n"
            "imul rax, %d \n"
            "add rbx, rax \n"
            "push rbx \n", offset * align + align, align);
    }
    else{          // primitive
        fprintf(write_ptr, 
            "mov rax, rbp \n"
            "sub rax, %d \n"
            "push rax \n\n", offset * align + align);
    }
}

int recall_or_update_variable(char* text, int* text_ptr, char* match){
    if(find_in_array(text, vars_global, vars_global_ptr) >= 0){
        char* left_brackets = read_chars(1, "#", 1);
        if(strcmp(left_brackets, "[") == 0){ // array
            ungetstring(left_brackets);
            read_chars(1, "#", 0);
            char* eq = read_chars(1, "#", 1);
            if(strcmp(eq, "=") == 0){        // update
                update_variable(text, 1);
                free(read_chars(0, ";", 0));
                assign_variable();
            }
            else{                            // recall
                ungetstring(eq);
                recall_variable(text, text_ptr, 1);
            }
            free(eq);
        }
        else{                                // primitive
            ungetstring(left_brackets);
            char* eq = read_chars(1, "#", 1);
            if(strcmp(eq, "=") == 0){        // update
                update_variable(text, -1);
                free(read_chars(0, ";", 0));
                assign_variable();
            }
            else{                            // recall
                ungetstring(eq);
                recall_variable(text, text_ptr, -1);
            }
            free(eq);
        }
        free(left_brackets);
        return 1;
    }
    //else if(find_function(text)){
    //}
    return 0;
}

void handle_function_or_variable(){
        free(read_chars(1, "#", 1));              // space
        char* var_name = read_chars("#", 1);
        char* next_token = read_chars("#", 1); // array
        if(strcmp(next_token, "[") == 0){
            char* size = read_chars("#", 1);
            char* right_brackets = read_chars("#", 1);
            mmap(atoi(size));
            set_variable(var_name);
            free(size);
            free(right_brackets);
        }
        else if(strcmp(next_token, "(") == 0){    // function
            fprintf(write_ptr, "\nfunc %d: \n", idem_key);
            free(read_chars(")", 0));
            free(read_chars("#", 1)); // {
            free(read_chars("}", 0));
            fprintf(write_ptr, "ret \n");
        }
        else{                                     // primitive
            ungetstring(next_token);
            char* eq = read_chars(1, "#", 1);          // =
            if(strcmp(eq, "=") == 0) set_variable(var_name);
            else set_function_parameter_variable(var_name);
            free(read_chars(0, match, 0));
            free(eq);
        }
        free(next_token);
        free(var_name);
}

int handle_token(char* text, int* text_ptr, char* match, int idem_key){
    if(strcmp(text, "print") == 0){
        free(read_chars(1, "#", 0));
        print_int(write_ptr);
        return 0;
    }
    else if(strcmp(text, "int") == 0){            // var or func
        handle_function_or_variable();
        return 0;
    }
    else if(strcmp(text, "if") == 0){
        free(read_chars("#", 1)); // (
        free(read_chars(")", 0));              
        cond_if(idem_key);
        fprintf(write_ptr, "cond_if%d: \n", idem_key);
        free(read_chars(1, "#", 0));              // {
        fprintf(write_ptr, 
            "jmp block%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else if(strcmp(text, "while") == 0){
        fprintf(write_ptr, 
            "jmp pre_while%d \n"
            "pre_while%d: \n", idem_key, idem_key);
        free(read_chars(1, "#", 0));              // (
        cond_while(idem_key);
        fprintf(write_ptr, "cond_while%d: \n", idem_key);
        free(read_chars(1, "#", 0));              // {
        fprintf(write_ptr, 
            "jmp pre_while%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else if(strcmp(text, "void") == 0){
    }
    else{
        return 0;
    }
}


char* read_chars(char* match, int term_early){
    char *text = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    text[0] = '\0';
    int text_ptr = 0;
    while(feof(read_ptr) == 0){
        int cur = fgetc(read_ptr);
        //fprintf(write_ptr, "; text: %s \n", text);
        //fprintf(write_ptr, "; -------------- \n");
        if(strcmp(text, match) == 0 || (term_early && is_changed)){ // exit early
            ungetc(cur, read_ptr);
            return text;
        }
        else if(text[0] && strchr("({[", text[0]) != NULL){                          // match left delimiter
            ungetc(cur, read_ptr);
            match_char(text);
            text_ptr = 0;
            text[0] = '\0';
        }
        else if(is_changed){                                                         // text changing
            ungetc(cur, read_ptr);
            if(store_number(text, &text_ptr) || recall_or_update_variable(text, &text_ptr, match)) {}
            else if(
                handler_math_or_bool_operator(text, &text_ptr, match) ||
                handle_token(text, &text_ptr, match, counter++)
            ){
                return text;
            }
            text_ptr = 0;
            text[0] = '\0';
        }
        else{
            text[text_ptr++] = cur;
            text[text_ptr] = '\0';
        }
    }
    if(feof(read_ptr) != 0){                                                         // store suffix
        store_number(text, &text_ptr);
    }
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

    free(read_chars(0, ";", 0));

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

