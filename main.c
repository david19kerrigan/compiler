#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(){
    FILE *fptr = fopen("input.c", "r");
    int sum = 0;
    while(1){
        char in = fgetc(fptr);
        if(strchr("0123456789", in)){
            sum += (in - '0');
        } else if(strchr("abcdefghijklmnopqrstuvwxyz", in)){
        }
        //printf("%c\n", in);
        if(feof(fptr) > 0) break;
    }
    printf("%d\n", sum);
    pclose(fptr);
    return 0;
}
