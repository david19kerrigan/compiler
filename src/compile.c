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

// return should_terminate_early
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
        free(read_token());
        mul();
        return 0;
    }
    else if(strcmp(text, "/") == 0){
        read_until_token(";");
        idiv();
        return 0;
    }
    else if(strcmp(text, ">") == 0){
        read_until_token(";");
        greater();
        return 0;
    }
    else if(strcmp(text, "<") == 0){
        read_until_token(";");
        less();
        return 0;
    }
    else if(strcmp(text, "==") == 0){
        read_until_token(";");
        equal();
        return 0;
    }
    else if(strcmp(text, "!=") == 0){
        read_until_token(";");
        equal();
        return 0;
    }
    else if(strcmp(text, "||") == 0){
        read_until_token(";");
        or();
        return 1;
    }
    else if(strcmp(text, "&&") == 0){
        read_until_token(";");
        and();
        return 1;
    }
    else return 0;
}

int is_num_char(char in){
    return strchr("0123456789", in) != NULL;
}

int is_num_str(char* text, int text_ptr){
    for(int i = 0; i < text_ptr; ++i){
        if(!is_num_char(text[i])){
            return 0;
        }
    }
    return 1;
}

int is_operator(char in){
    return strchr("/*-+=&|<>", in) != NULL;
}

int is_letter(char in){
    return strchr("abcdefghijklmnopqrstuvwxyz", in) != NULL;
}

int get_type(char in){
    if(is_num_char(in)) return 0;
    else if(is_operator(in)) return 1;
    else if(is_letter(in)) return 2;
    else return 3;
}

int check_should_terminate(char* text, int text_ptr, char cur){
    if((text_ptr > 0 && get_type(text[text_ptr-1]) != get_type(cur)) || // changing type
        (text_ptr == 1 && strchr(";\n ([{", text[0]) != NULL)) return 1; // bracket or newline
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

void store_number(char* text, int text_ptr){
    if(is_num_str(text, text_ptr))
        fprintf(write_ptr, "push %s \n", text);
    }
}

void set_variable(char* text){
    fprintf(write_ptr, "; set |%s|\n", text);
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
        char* left_brackets = read_chars("#");
        if(strcmp(left_brackets, "[") == 0){ // array
            ungetstring(left_brackets);
            read_chars("#");
            char* eq = read_chars("#");
            if(strcmp(eq, "=") == 0){        // update
                update_variable(text, 1);
                free(read_chars(";"));
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
            char* eq = read_chars("#");
            if(strcmp(eq, "=") == 0){        // update
                update_variable(text, -1);
                free(read_chars(";"));
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
    // else if(find_function(text)){
    // }
    return 0;
}
*/

void handle_function_or_variable(char* match, int idem_key){
        check_next_word(" ");
        char* var_name = read_token();
        char* next_token = read_token();
        /*
        if(strcmp(next_token, "[") == 0){         // array
            char* size = read_chars("#");
            char* right_brackets = read_chars("#");
            mmap(atoi(size));
            set_variable(var_name);
            free(size);
            free(right_brackets);
        }
        else if(strcmp(next_token, "(") == 0){    // function
            fprintf(write_ptr, "\nfunc %d: \n", idem_key);
            free(read_chars(")"));
            check_next_word("{");
            free(read_chars("}"));
            fprintf(write_ptr, "ret \n");
        }
        */
        else if(strcmp(next_token, " ") == 0){                                     // primitive
            ungetstring(next_token);
            char* eq = read_token();
            if(strcmp(eq, "=") == 0) set_variable(var_name);
            //else set_function_parameter_variable(var_name);
            read_until_char(match)); // is this needed?
            free(eq);
        }
        free(next_token);
        free(var_name);
}

void handle_token(char* text){
    if(strcmp(text, "print") == 0){
        check_next_word("(");
        read_until_token(")");
        print_int(write_ptr);
    }
    else if(strcmp(text, "int") == 0){            // var or func
        handle_function_or_variable(match, idem_key);
    }
    /*
    else if(strcmp(text, "if") == 0){
        check_next_word("(");
        free(read_chars(")"));              
        cond_if(idem_key);
        fprintf(write_ptr, "cond_if%d: \n", idem_key);
        check_next_word("{");
        free(read_chars("}")); 
        fprintf(write_ptr, 
            "jmp block%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else if(strcmp(text, "while") == 0){
        fprintf(write_ptr, 
            "jmp pre_while%d \n"
            "pre_while%d: \n", idem_key, idem_key);
        check_next_word("(");
        free(read_chars(")"));
        cond_while(idem_key);
        fprintf(write_ptr, "cond_while%d: \n", idem_key);
        check_next_word("{");
        free(read_chars("}")); 
        fprintf(write_ptr, 
            "jmp pre_while%d \n"
            "block%d: \n", idem_key, idem_key);
        return 1;
    }
    else if(strcmp(text, "void") == 0){
    }
    */
}

void check_next_word(char* text){
    char* next = read_token();
    if(strcmp(next, text) != 0){
        free(next);
        printf("Expected %s found %s\n", text, next);
        exit(0);
    }
}

void read_until_token(char* text){
    while(feof(read_ptr) == 0){
        char* token = read_token();
        //fprintf(write_ptr, "; token: %s ; match: %s\n", token, text);
        if(handle_operator(token, text) || strcmp(text, token) == 0){ // terminate early OR match found
            free(token);
            break;
        }
        handle_token(token);
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
            store_number(text, text_ptr);
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

