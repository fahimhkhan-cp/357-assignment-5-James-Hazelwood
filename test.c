#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("this is a test\n");

    if(argc > 1){
        printf("%s", argv[1]);
    }
}