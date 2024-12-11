#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <compile.h>

static FILE* read_ptr = NULL;
static FILE* write_ptr = NULL; 
static char** vars = NULL;
static int vars_ptr = 0;
static int level = 0;
static int counter = 0;
static const char* build_dir = "build/";
static const char* src_dir = "src/";
static const int DEFAULT_SIZE = 32;
static const int align = 32;

void match_char(char* text){
    if(strcmp(text, "(") == 0) free(read_chars(0, ")", 0));
    else if(strcmp(text, "{") == 0) free(read_chars(0, "}", 0));
}

int find_variable(char* text){
    int offset = -1;
    for(int i = 0; i < vars_ptr; ++i){
        if(strcmp(vars[i], text) == 0) offset = i;
    }
    return offset;
}

void ungetstring(char* text){
    for(int i = 0; i < strlen(text); ++i){
        ungetc(text[i], read_ptr);
    }
}

void recall_variable(char* text, int* text_ptr){
    if(*text_ptr > 0 && is_letter(text[*text_ptr-1])){
        // fprintf(write_ptr, "; recall |%s|\n", text);
        int offset = find_variable(text);
        if(offset < 0) return;
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

void update_variable(char* text){
    fprintf(write_ptr, "; update |%s|\n", text);
    int offset = find_variable(text);
    if(offset < 0) return;
    fprintf(write_ptr,
        "pop rax \n"
        "mov [rbp-%d], rax \n\n", offset * align + align);
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

void print_int(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rbx \n"
        "lea rdi, [rbx] \n"
        "call print_int \n\n");
}

void cond_if(FILE* write_ptr, int idem_key){
    fprintf(write_ptr, 
        "pop rax \n"
        "cmp rax, 1 \n"
        "je cond_if%d \n"
        "jne block%d \n\n", idem_key, idem_key);
}

void cond_while(FILE* write_ptr, int idem_key){
    fprintf(write_ptr, 
        "pop rax \n"
        "cmp rax, 1 \n"
        "je cond_while%d \n"
        "jne block%d \n\n", idem_key, idem_key);
}

void or(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "or rax, rbx \n"
        "push rax \n\n");
}

void and(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "and rax, rbx \n"
        "push rax \n\n");
}

void greater(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "cmp rax, rbx \n"
        "setg al \n"
        "push rax \n\n");
}

void less(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "cmp rax, rbx \n"
        "setl al \n"
        "push rax \n\n");
}

void unequal(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "xor al, al"
        "setnz al \n"
        "push rax \n\n");
}

void equal(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "cmp rax, rbx \n"
        "setz al \n"
        "push rax \n\n");
}

void add(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rax \n"
        "pop rbx \n"
        "add rax, rbx \n"
        "push rax \n\n");
}

void sub(FILE* write_ptr){
    fprintf(write_ptr, 
        "pop rbx \n"
        "pop rax \n"
        "sub rax, rbx \n"
        "push rax \n\n");
}

void mul(FILE* write_ptr){
    fprintf(write_ptr,
        "pop rax \n"
        "pop rbx \n"
        "imul rax, rbx \n"
        "push rax \n\n");
}

void idiv(FILE* write_ptr){
    fprintf(write_ptr,
        "pop rbx \n"
        "pop rax \n"
        "idiv rbx \n"
        "push rax \n\n");
}

void recall_or_update_variable(char* text, int* text_ptr, char* match){
    if(find_variable(text) >= 0){
        char* eq = read_chars(1, "#", 1);
        if(strcmp(eq, "=") == 0){
            free(read_chars(0, match, 0));
            update_variable(text);
        }
        else{
            ungetstring(eq);
            recall_variable(text, text_ptr);
        }
        free(eq);
    }
}

int handle_function(char* text, int* text_ptr, char* match, int idem_key){
    if(strcmp(text, "print") == 0){
        free(read_chars(0, match, 0));
        print_int(write_ptr);
        return 1;
    }
    else if(strcmp(text, "int") == 0){
        free(read_chars(1, "#", 0)); // space
        char* var_name = read_chars(1, "#", 0);
        free(read_chars(1, "#", 0)); // =
        free(read_chars(0, match, 0));
        set_variable(var_name);
        free(var_name);
        return 0;
    }
    else if(strcmp(text, "if") == 0){
        free(read_chars(1, "#", 0)); // (
        cond_if(write_ptr, idem_key);
        fprintf(write_ptr, "cond_if%d: \n", idem_key);
        free(read_chars(1, "#", 0)); // {
        fprintf(write_ptr, 
            "jmp block%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else if(strcmp(text, "else") == 0){
    }
    else if(strcmp(text, "while") == 0){
        fprintf(write_ptr, "jmp pre_while%d \n", idem_key);
        fprintf(write_ptr, "pre_while%d: \n", idem_key);
        free(read_chars(1, "#", 0)); // (
        cond_while(write_ptr, idem_key);
        fprintf(write_ptr, "cond_while%d: \n", idem_key);
        free(read_chars(1, "#", 0)); // {
        fprintf(write_ptr, 
            "jmp pre_while%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else{
        return 0;
    }
}

int handle_operator(char* text, int* text_ptr, char* match){
    if(strcmp(text, "-") == 0){
        free(read_chars(0, match, 0));
        sub(write_ptr);
        return 1;
    }
    else if(strcmp(text, "+") == 0){
        free(read_chars(0, match, 0));
        add(write_ptr);
        return 1;
    }
    else if(strcmp(text, "*") == 0){
        free(read_chars(1, match, 0));
        mul(write_ptr);
        return 0;
    }
    else if(strcmp(text, "/") == 0){
        free(read_chars(1, match, 0));
        idiv(write_ptr);
        return 0;
    }
    else if(strcmp(text, ">") == 0){
        free(read_chars(1, match, 0));
        greater(write_ptr);
        return 0;
    }
    else if(strcmp(text, "<") == 0){
        free(read_chars(1, match, 0));
        less(write_ptr);
        return 0;
    }
    else if(strcmp(text, "==") == 0){
        free(read_chars(1, match, 0));
        equal(write_ptr);
        return 0;
    }
    else if(strcmp(text, "!=") == 0){
        free(read_chars(1, match, 0));
        equal(write_ptr);
        return 0;
    }
    else if(strcmp(text, "||") == 0){
        free(read_chars(0, match, 0));
        or(write_ptr);
        return 1;
    }
    else if(strcmp(text, "&&") == 0){
        free(read_chars(0, match, 0));
        and(write_ptr);
        return 1;
    }
    else return 0;
}

char* read_chars(int length, char* match, int term_early){
    char *text = (char*) malloc(sizeof(char) * DEFAULT_SIZE);
    text[0] = '\0';
    int text_ptr = 0;
    ++level;
    while(feof(read_ptr) == 0){
        int cur = fgetc(read_ptr);
        //fprintf(write_ptr, "; cur : %s \n", &cur);
        //fprintf(write_ptr, "; text: %s \n", text);
        //fprintf(write_ptr, "; lvl :[%d] \n", level);
        //fprintf(write_ptr, "; -------------- \n");
        if(strcmp(text, match) == 0 || (text_ptr > 0 && get_type(text[text_ptr-1]) != get_type(cur) && term_early && length > 0 && --length == 0)){
            ungetc(cur, read_ptr);
            --level;
            return text;
        }
        else if(text[0] && strchr("({", text[0]) != NULL){
            ungetc(cur, read_ptr);
            match_char(text);
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
            recall_or_update_variable(text, &text_ptr, match);
            if(
                (length > 0 && --length == 0) ||
                handle_operator(text, &text_ptr, match) ||
                handle_function(text, &text_ptr, match, counter++)
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
        free(read_chars(0, ";", 0));
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

