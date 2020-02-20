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
//pid_t pId = 1;
void exitf(void){
     exit(0);
     //kill(pId, )
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
    //int status; 
    pid_t pid = fork();
    if (pid < -1) {
        printf("Error, cannot fork\n");
    } else if (pid == 0) {
            //printf("[C] I am the child\n");
        execvp(argv[0], argv);
        printf("icsh: command not found: %s\n", argv[0]);
        return status;
    } else {
            //printf("[P] I'm waiting for my child\n");
        wait(&status);
        if (WIFEXITED(status))  {
            int exit_status = WEXITSTATUS(status);      
            printf("Exit status of the child was %d\n", exit_status);
        }
    }
    return status;
}
int executef(char **argv){   
    if (argv[0] == NULL) {
        return 1;
    }
    if (strcmp(argv[0], "exit") == 0){
        printf("%s\n", argv[0]);
        exitf();
    }
    //if(argv[0] == "echo" && argv[1] == "$?"){
    if(strcmp(argv[0],"echo") == 0 && strcmp(argv[1], "$?") == 0){
        /*if (status == 9){
            printf("%d\n",0);
            return 1;
        }else{
            printf("%d\n",status);
            return 1;
        }
        //return 1;*/
        printf("%d\n",status);
        return 1;
    }
    return processf(argv);
}
void ctrlcf(){
    ;
}
/*void ctrlzf(int signal){
    if(signal==SIGTSTP){
        
    }
}*/
void icshLoop(){
    // print loop icsh> here
    //pId = getpid;
    while(1){
        signal(SIGINT, ctrlcf);
        signal(SIGSTOP, SIG_IGN);
        printf("icsh> ");
        char *r = readf();
        char **split = splitf(r);
        status = executef(split);
        //int exit_status = 1;
    }
}
int main(void){
    icshLoop();
}