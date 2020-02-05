//Natthakan Euaumpon 6081213
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<ctype.h>

int status = 1;
int exitf(void){
    return 0;
}
char **splitf(char *argv){
    int i =0;
    const char s[10] = " \t\r\n\a";
    char **l = malloc(sizeof(char*)*256);
    char *w;
    w = strtok(argv,s);
    while(w != NULL){
        l[i]= w;
        i++;
        w = strtok(NULL, s);
    }
    l[i] = NULL;
    return l;
}

char *readf(void){
  char *l = NULL;
  size_t bufsize = 0; 
  getline(&l, &bufsize, stdin);
  return l;
}
int processf(char **argv){
    int status; 
    pid_t pid = fork();
    if (pid < -1) {
        printf("Error, cannot fork\n");
    } else if (pid == 0) {
            //printf("[C] I am the child\n");
        execvp(argv[0], argv);
        printf("icsh: command not found: %s\n", argv[0]);
    } else {
            //printf("[P] I'm waiting for my child\n");
        wait(&status);
        if (WIFEXITED(status))  {
            int exit_status = WEXITSTATUS(status);         
            printf("Exit status of the child was %d\n", exit_status); 
        }
    }
    return 9;
}
int executef(char **argv){   
    if (argv[0] == NULL) {
        return 1;
    }
    if(strcmp(argv[0], "exit") == 0){
        return exitf();
    }
    //if(argv[0] == "echo" && argv[1] == "$?"){
    if(strcmp(argv[0],"echo") == 0 && strcmp(argv[1], "$?") == 0){
        printf("%d\n",status);
        return 1;
    }
    return processf(argv);
}
void ctrlcf(){
    ;
}
// void ctrlzFunction(){
//     ;
// }
void icshLoop(){
    // print loop icsh> here
    while(status){
    signal(SIGINT, ctrlcf);
    // signal(SIGSTOP, ctrlzFunction);
    printf("icsh> ");
    char *r = readf();
    char **split = splitf(r);
    status = executef(split);
    }
}
int main(void){
    icshLoop();
}