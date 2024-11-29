#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <compile.h>

static FILE *read_ptr = NULL;
static FILE *write_ptr = NULL; 
static char **vars = NULL;
static int vars_ptr = 0;
static const char *build_dir = "build/";
static const char *src_dir = "src/";
static const int DEFAULT_SIZE = 32;

void recall_variable(char* var_name){
    if(vars_ptr > 0){
        int offset;
        for(int i = 0; i < vars_ptr; ++i){
            if(strcmp(vars[i], var_name) == 0) offset = i;
        }
        fprintf(write_ptr,
            "mov rax, [rbp-%d] \n", offset * 16 + 16,
            "push rax \n\n");
    }
}

void set_variable(char* text, int text_ptr){
    if(text_ptr > 0){
        fprintf(write_ptr,
            "pop rax \n"
            "mov [rbp-%d], rax \n\n", vars_ptr * 16 + 16);
        text[text_ptr] = '\0';
        vars[vars_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
        strcpy(vars[vars_ptr++], text);
        text_ptr = 0;
    }
}

void store_number(char* num, int num_ptr){
    if(num_ptr > 0){
        num[num_ptr] = '\0';
        fprintf(write_ptr, "push %s \n", num);
        num_ptr = 0;
    }
}

int is_num(char in){
    return strchr("0123456789", in) != NULL;
}

int is_operator(char in){
    return strchr("/*-+", in) != NULL;
}

int is_letter(char in){
    return strchr("abcdefghijklmnopqrstuvwxyz", in) != NULL;
}

void print_int(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "lea rdi, [rbx] \n"
        "call print_int \n\n");
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

int handle_math_operator(int cur, char* num, int num_ptr){
    if(cur == '-'){
        read_chars(0);
        sub();
        return 1;
    }
    else if(cur == '+'){
        read_chars(0);
        add();
        return 1;
    }
    else if(cur == '*'){
        read_chars(1);
        mul();
        return 0;
    }
    else if(cur == '/'){
        read_chars(1);
        idiv();
        return 0;
    }
}

void read_chars(int length){
    char *text = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    char *num = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    int text_ptr = 0;
    int num_ptr = 0;
    while(feof(read_ptr) == 0){
        int cur = fgetc(read_ptr);
        fprintf(write_ptr, "; cur %s \n", &cur);
        if(is_num(cur)){
            num[num_ptr++] = cur;
        }
        else if(is_letter(cur)){
            text[text_ptr++] = cur;
        }
        else{
            if(length > 0 && --length == 0){
                ungetc(cur, read_ptr);
                store_number(num, num_ptr);
                recall_variable(text);
                return;
            }
            else if(cur == '('){
                store_number(num, num_ptr);
                read_chars(0);
                text_ptr = '\0';
                if(strcmp(text, "print") == 0){
                    print_int();
                }
            }
            else if(cur == ')'){
                store_number(num, num_ptr);
                return;
            }
            else if(is_operator(cur)){
                store_number(num, num_ptr);
                recall_variable(text);
                if(handle_math_operator(cur, num, num_ptr)) return;
            }
            else if(cur == '='){
                read_chars(0);
                set_variable(text, text_ptr);
                return;
            }
            else if(strchr(";\n ", cur)){
                return;
            }
        }
    }
    if(feof(read_ptr) != 0){
        store_number(num, num_ptr);
    }
    free(text);
    free(num);
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
    vars = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);

    fprintf(write_ptr,
        "align 16 \n"
        "global _start \n" 
        "extern print_int \n" 
        "extern int_to_string \n" 
        "extern exit \n \n" 
        "section .text \n" 
        "_start: \n"); 

    while(feof(read_ptr) == 0){
        read_chars(0);
    }

    fprintf(write_ptr, "call exit\n");

    for(int i = 0; i < vars_ptr; ++i){
        free(vars[i]);
    }
    free(vars);

    pclose(read_ptr);
    fclose(write_ptr);
}

int main(int argc, char* argv[]){
    compile(argv[1]);
    return 0;
}

