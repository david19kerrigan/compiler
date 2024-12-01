void compile(char* input_file);
int is_num(char in);
int is_operator(char in);
void recall_variable(char* text, int* text_ptr);
int find_variable(char* var_name);
void mul();
void add();
void sub();
void idiv();
void read_chars(int length);
int handle_operator(char* text, int* text_ptr);
void print_int();
int is_num(char in);
int is_operator(char in);
int is_letter(char in);
int get_type(char in);

