#define _GNU_SOURCE

#include "cgi_like.h"
#include "net.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define RESPONCE_BUFFER_SIZE 128

void send_responce(int client_fd, const char *status, const char *content_type, const char *content, size_t content_length){
   dprintf(client_fd, "HTTP/1.0 %s\r\n", status);
   dprintf(client_fd, "Content-Type: %s\r\n", content_type);
   dprintf(client_fd, "Content-Length: %zu\r\n", content_length);
   dprintf(client_fd, "\r\n");

   if (content && content_length > 0){
      write(client_fd, content, content_length);
      write(client_fd, "\n", 1);
   }
}

void send_error_responce(int client_fd, const char *status, const char *message){
   char body[RESPONCE_BUFFER_SIZE];
   snprintf(body, sizeof(body), "<html><body><h1>%s</h1><p>%s</p></body></html>", status, message);
   send_responce(client_fd, status, "text/html", body, strlen(body));
}

size_t file_size(char name[]) {

   // make stat struct to get size 
   struct stat file_size;
   // st_size > size

   if (stat(name, &file_size) < 0){
      return 0;
   }

   return file_size.st_size;

}

void handle_request(int nfd) {
   FILE *network = fdopen(nfd, "r");
   char *line = NULL;
   size_t size;
   ssize_t num;
   pid_t pid;

   if (network == NULL)
   {
      perror("fdopen");
      close(nfd);
      return;
   }


   while ((num = getline(&line, &size, network)) >= 0) {    
      pid = fork();
      if (pid < 0){
         perror("forking didn't work");
         break;

      } else if (pid == 0){
         // child process
         printf("Message received from client: %s", line);

         char *first = strsep(&line, " ");
         char *second = strsep(&line, " ");
         char *third = strsep(&line, " ");
         char *extra = strsep(&line, " ");

         // ingore some requests that don't begin with GET or HEAD
         if(!(strcmp("HEAD", first) == 0 || strcmp("GET", first) == 0)){
            free(line);
            free(first);
            fclose(network);
            exit(0);
         }

         // due to input being /whatever
         second++;
         
         printf("%s %s %s\n", first, second, third);

         // make sure request is in correct format
         if(extra != NULL || third == NULL){
            while((extra = strsep(&line, " ")) != NULL){
               continue;
            } 
            
            send_error_responce(nfd, "400", "Not the correct number of args");
            free(line);
            free(first);
            fclose(network);
            exit(1);
         }

         // check .. 
         if(second[0] == '.' && second[1] == '.'){
            
            send_error_responce(nfd, "500", "dont do that bro :(");
            free(line);
            free(first);
            fclose(network);
            exit(1);
         }

         

         if(strcmp("HEAD", first) == 0) {

            // get size
            size_t size = file_size(second);

            if(size == 0){
               // site not found
               
               send_error_responce(nfd, "404", "Not Found");
               free(line);
               free(first);
               fclose(network);
               exit(1);
            }

            send_responce(nfd, "200 OK", "text/html", NULL, size);
            free(line);
            free(first);
            fclose(network);
            exit(0);

         } else if(strcmp("GET", first) == 0){
            
            FILE *file;
            size_t size;
            if(strncmp(second, "cgi-like", 8) == 0){

               int i;
               for(i = 0; i < 9; i++){
                  second++;
               }
               
               char file_name[20];
               pid_t parent_pid = getpid();
               sprintf(file_name, "%d.txt", parent_pid);

               pid_t pid2 = fork();

               if(pid2 < 0){
                  send_error_responce(nfd, "500", "error with cgi-like");
                  free(line);
                  free(first);
                  fclose(network);
                  exit(1);

               } else if(pid2 == 0) {

                  int value;
                  value = cgi_like(second, file_name);
                  exit(value);
                  
               } else {
                  int status;
                  waitpid(pid2, &status, 0);

                  if(WEXITSTATUS(status) > 0){
                     printf("%i", WEXITSTATUS(status));
                     send_error_responce(nfd, "500", "error with cgi-like");
                     free(line);
                     free(first);
                     fclose(network);
                     exit(1);
                  }

                  // get size
                  size = file_size(file_name);

                  if(size == 0){
                     // site not found
                     send_error_responce(nfd, "404", "Not Found (size)");
                     free(line);
                     free(first);
                     fclose(network);
                     exit(1);
                  }

                  file = fopen(file_name, "r");
               }
               

            } else {

               // get size
               size = file_size(second);

               if(size == 0){
                  // site not found
                  send_error_responce(nfd, "404", "Not Found (SIZE)");
                  free(line);
                  free(first);
                  fclose(network);
                  exit(1);
               }

               file = fopen(second, "r");
            }

            char content[size];
            if(file == NULL){
               fclose(file);
               send_error_responce(nfd, "404", "FILE Not Found NULL FILE");
               free(line);
               free(first);
               fclose(network);
               exit(1);
            }

            int read;

            if((read = fread(content, 1, size, file)) < size){
               fclose(file);
               send_error_responce(nfd, "404", "Cant Read File");
               free(line);
               free(first);
               fclose(network);
               exit(1);
            }
            
            send_responce(nfd, "200 OK", "text/html", content, size);

            fclose(file);
            free(line);
            free(first);
            fclose(network);
            exit(0);
         }
         
         send_error_responce(nfd, "400", "First arg needs to be either HEAD or GET");
         free(line);
         free(first);
         fclose(network);
         exit(1);
      } 
   }

   fclose(network);
}

void run_service(int fd)
{
   while (1)
   {
      int nfd = accept_connection(fd);
      if (nfd != -1)
      {
         printf("Connection established\n");
         handle_request(nfd);
         printf("Connection closed\n");
      }
   }
}

int main(void)
{
   int port; 
   printf("ENTER PORT: ");
   scanf("%d", &port);

   if(port > 65535 || port < 1024) {
      perror("Invalid port");
      return 1;
   }

   int fd = create_service(port);

   if (fd == -1)
   {
      perror(0);
      exit(1);
   }

   printf("listening on port: %d\n", port);
   run_service(fd);
   close(fd);

   return 0;
}
