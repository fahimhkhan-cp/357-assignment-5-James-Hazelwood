#define _GNU_SOURCE

#include "cgi_like.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int cgi_like(char *command, char *file_name){

    char *name = strsep(&command, "?");

    if(strlen(name) >= 2 && name[0] == '.' && name[1] == '.'){
        return 1;
    }

    Array *array = malloc(sizeof(Array));
    array->strings = malloc(sizeof(char *) * 5);
    array->strings[0] = strdup(name);
    array->items = 1;
    array-> capacity = 5;

    char *args;
    while((args = strsep(&command, "&")) != NULL){

        if(array->items >= array->capacity){
            array->capacity += 1;
            array->strings = realloc(array->strings, sizeof(char *) * array->capacity);
        }

        array->strings[array->items] = strdup(args);
        array->items += 1;
    }

    // add NULL
    if(array->items >= array->capacity){
        array->capacity += 1;
        array->strings = realloc(array->strings, sizeof(char *) * array->capacity);
    }

    array->strings[array->items] = NULL;
    array->items += 1;

    int file = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(file == -1){
        // free memory
        int i;
        for(i = 0; i < array->items; i++){
            free(array->strings[i]);
        }
        free(array->strings);
        free(array);
        perror("Error opening file");
        return 1;
    }
    
    // make stdout now go to file descriptor
    if (dup2(file, STDOUT_FILENO) == -1) {
        // free memory
        int i;
        for(i = 0; i < array->items; i++){
            free(array->strings[i]);
        }
        free(array->strings);
        free(array);
        perror("dup2 failed");
        return 1;
    }

    close(file);

    char path[124] = "./";
    strcat(path, name);

    int read;
    printf("path %s", path);

    read = execvp(path, array->strings);
    if (read == -1) {
        // free memory
        int i;
        for(i = 0; i < array->items; i++){
            free(array->strings[i]);
        }
        free(array->strings);
        free(array);
        perror("exec failed");
        return 1;
    }

    // free memory
    int i;
    for(i = 0; i < array->items; i++){
        free(array->strings[i]);
    }
    free(array->strings);
    free(array);
    return 0;
}


    

    



    
