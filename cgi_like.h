#ifndef CGI_LIKE_H
#define CGI_LIKE_H


typedef struct Array Array;

struct Array {
    char **strings;
    int capacity;
    int items; 
};

int cgi_like(char *command, char *pid);

#endif