void compile(char* input_file);
int is_num(char in);
int is_operator(char in);
void recall_variable(char* text, int* text_ptr, FILE* file_ptr);
int find_variable(char* var_name);
void mul();
void add();
void sub();
void idiv();
char* read_chars(int length, char* match, FILE* file_ptr);
int handle_operator(char* text, int* text_ptr, char* match, FILE* file_ptr);
void print_int();
int is_num(char in);
int is_operator(char in);
int is_letter(char in);
int get_type(char in);

