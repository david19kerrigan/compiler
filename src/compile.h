void compile(char* input_file);
int is_num(char in);
int is_operator(char in);
void recall_variable(char* text, int* text_ptr, int offset);
int find_variable(char* var_name);
void mul();
void add();
void sub();
void idiv();
char* read_chars(int length, char* match, int term_early);
int handle_token(char* text, int* text_ptr, char* match, int idem_key);
void print_int();
int is_num(char in);
int is_operator(char in);
int is_letter(char in);
int get_type(char in);

