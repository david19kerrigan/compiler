#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    FILE *fptr = fopen("input.c", "r");
    int *stack[] = malloc(32);
    int sum = 0;
    while(feof(fptr) == 0){
        char in = fgetc(fptr);
        if(strchr("0123456789", in)){
        } else if(strchr("abcdefghijklmnopqrstuvwxyz", in)){
        } else if(in == '+'){
            sum += (in - '0');
        } else if(in == '-'){
        } else if(in == '*'){
        } else if(in == '/'){
        }
    }
    printf("%d\n", sum);
    pclose(fptr);
    return 0;
}
