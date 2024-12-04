#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <compile.h>

static FILE *read_ptr = NULL;
static FILE *write_ptr = NULL; 
static char **vars = NULL;
static int vars_ptr = 0;
static int level = 0;
static const char *build_dir = "build/";
static const char *src_dir = "src/";
static const int DEFAULT_SIZE = 32;
static const int align = 32;

void recall_variable(char* text, int* text_ptr){
    if(*text_ptr > 0 && is_letter(text[*text_ptr-1])){
        // fprintf(write_ptr, "; recall |%s|\n", text);
        int offset = -1;
        for(int i = 0; i < vars_ptr; ++i){
            if(strcmp(vars[i], text) == 0) offset = i;
        }
        if(offset == -1) return;
        fprintf(write_ptr,
            "mov rax, [rbp-%d] \n"
            "push rax \n\n", offset * align + align);
        *text_ptr = 0;
        text[0] = '\0';
    }
}

void set_variable(char* text){
    // fprintf(write_ptr, "; set |%s|\n", text);
    fprintf(write_ptr,
        "pop rax \n"
        "mov [rbp-%d], rax \n\n", vars_ptr * align + align);
    vars[vars_ptr] = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    strcpy(vars[vars_ptr++], text);
}

void store_number(char* num, int* num_ptr){
    if(*num_ptr > 0 && is_num(num[*num_ptr-1])){
        fprintf(write_ptr, "push %s \n", num);
        *num_ptr = 0;
        num[0] = '\0';
    }
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
    else return 6;
}

void print_int(){
    fprintf(write_ptr, 
        "pop rbx \n"
        "lea rdi, [rbx] \n"
        "call print_int \n\n");
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
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setg al \n"
        "push rax \n\n");
}

void less(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setl al \n"
        "push rax \n\n");
}

void unequal(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setz al \n"
        "push rax \n\n");
}

void equal(){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setnz al \n"
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

int handle_function(char* text, int* text_ptr){
    if(strcmp(text, "print") == 0){
        free(read_chars(0));
        print_int();
        return 1;
    }
    else if(strcmp(text, "int") == 0){
        free(read_chars(1));
        char* var_name = read_chars(1);
        free(read_chars(1));
        free(read_chars(0));
        set_variable(var_name);
        free(var_name);
        return 0;
    }
    else if(strcmp(text, "if") == 0){
    }
    else if(strcmp(text, "while") == 0){
    }
    else return 0;
}

int handle_operator(char* text, int* text_ptr){
    if(strcmp(text, "-") == 0){
        free(read_chars(0));
        sub();
        return 1;
    }
    else if(strcmp(text, "+") == 0){
        free(read_chars(0));
        add();
        return 1;
    }
    else if(strcmp(text, "*") == 0){
        free(read_chars(1));
        mul();
        return 0;
    }
    else if(strcmp(text, "/") == 0){
        free(read_chars(1));
        idiv();
        return 0;
    }
    else if(strcmp(text, ">") == 0){
        free(read_chars(1));
        greater();
        return 0;
    }
    else if(strcmp(text, "<") == 0){
        free(read_chars(1));
        greater();
        return 0;
    }
    else if(strcmp(text, "==") == 0){
        free(read_chars(1));
        equal();
        return 0;
    }
    else if(strcmp(text, "!=") == 0){
        free(read_chars(1));
        equal();
        return 0;
    }
    else if(strcmp(text, "||") == 0){
        free(read_chars(0));
        or();
        return 1;
    }
    else if(strcmp(text, "&&") == 0){
        free(read_chars(0));
        and();
        return 1;
    }
    else return 0;
}

char* read_chars(int length){
    char *text = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    text[0] = '\0';
    int text_ptr = 0;
    ++level;
    while(feof(read_ptr) == 0){
        int cur = fgetc(read_ptr);
        //fprintf(write_ptr, "; cur : %s \n", &cur);
        //fprintf(write_ptr, "; text: |%s| \n", text);
        //fprintf(write_ptr, ";[lvl : %d] \n", level);
        //fprintf(write_ptr, "; -------------- \n");
        if(strcmp(text, "(") == 0){
            ungetc(cur, read_ptr);
            free(read_chars(0));
            if(length > 0 && --length == 0){
                --level;
                return text;
            }
            text_ptr = 0;
            text[0] = '\0';
        }
        else if(text_ptr > 0 && get_type(text[text_ptr-1]) != get_type(cur)){
            ungetc(cur, read_ptr);
            store_number(text, &text_ptr);
            recall_variable(text, &text_ptr);
            if(
                (length > 0 && --length == 0) ||
                strcmp(text, ")") == 0 || 
                strcmp(text, ";") == 0 ||
                strcmp(text, " ") == 0 ||
                strcmp(text, "\n") == 0 ||
                handle_operator(text, &text_ptr) ||
                handle_function(text, &text_ptr)
            ){
                --level;
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
    if(feof(read_ptr) != 0){
        store_number(text, &text_ptr);
    }
    --level;
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
    vars = (char**) malloc(sizeof(char*) * DEFAULT_SIZE);

    fprintf(write_ptr,
        "global _start \n" 
        "extern print_int \n" 
        "extern int_to_string \n" 
        "extern exit \n \n" 
        "section .text \n" 
        "_start: \n"
        "mov rbp, rsp \n"); 

    while(feof(read_ptr) == 0){
        free(read_chars(0));
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

