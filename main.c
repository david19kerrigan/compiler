#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int DEFAULT_ARR_SIZE = 32;

int main(){
    FILE *fptr = fopen("input.c", "r");
    int stack_size = DEFAULT_ARR_SIZE;
    int *stack = malloc(stack_size * sizeof(int));
    int stack_ind = 0;
    int sum = 0;
    while(feof(fptr) == 0){
        char in = fgetc(fptr);
        if(strchr("0123456789", in)){
            stack[stack_ind++] = in - '0';
        } else if(strchr("abcdefghijklmnopqrstuvwxyz", in)){
        } else if(in == '+'){
            //sum += (in - '0');
        } else if(in == '-'){
        } else if(in == '*'){
        } else if(in == '/'){
        }
        if(stack_ind == stack_size-1){
            stack = realloc(stack, stack_size * 2 * sizeof(int));
            stack_size *= 2;
        }
    }
    printf("\n");
    for(int i = 0; i < stack_ind; ++i){
        printf("%d ", stack[i]);
    }
    free(stack);
    printf("%d\n", sum);
    pclose(fptr);
    return 0;
}
