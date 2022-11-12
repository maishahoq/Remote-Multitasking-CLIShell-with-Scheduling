#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/wait.h>

#define _GNU_SOURCE

#define MAX_SIZE 100
#define MAXLIST 100
#define MAXCOM 100



// Greeting shell during startup
void init_shell()
{
    
    printf("\n\n\n\n******************************************     \n");
    printf("\n\n\n**** Bhavicka and Maisha's SHELL ****\n");
    printf("\n\n\n\n******************************************  \n");
    printf("Enter \"help\" to get help and \"exit\" to exit the shell\n");
    char* username = getenv("USER");
    printf("\n\n\n USER: @  %s", username);
    printf("\n");
    sleep(1);
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char* username = getenv("USER");
    printf("\n   Dir : %s : @%s  ", cwd, username);
}


void help()
{
    printf("Welcome to our shell! You can enter single commands or piped commands here. A non-exhaustive list of commands includes:\nls\npwd\nman\ndate\nmkdir\nrmdir\nrm\ncp\nmv\nps\n");
    printf("You can also try out commands with single (...|...), double (...|...|...) or triple (...|...|...|...) pipes.\n");
    printf("\nPLEASE NOTE: All combinations work for single commands, but if you want to use pipes you must use single word non-combination commands. This means a|b, not a | b or a| b or a |b or a -c|b etc. Some examples you could try out are:\n");
    printf("pwd|wc\nls|wc\nls|sort\nls|more|wc\nls|more|sort|wc\n\n or any such commands in a similar vein");
}


//function to compare and limit user input of commands

// int stringComp(char * inputString)
//{
//    if(strcmp("ls|wc", inputString)==0 || strcmp("pwd|wc", inputString)==0 || strcmp("ls|more", inputString)==0 || strcmp("ps|more", inputString)==0 || strcmp("ls|sort", inputString)==0 || strcmp("ps|sort", inputString)==0 || strcmp("ps|sort|wc", inputString)==0 || strcmp("ls|sort|wc", inputString)==0 || strcmp("ps|more|wc", inputString)==0 || strcmp("ls|more|wc", inputString)==0 || strcmp("ls|more|sort|wc", inputString)==0 || strcmp("ls|sort|more|wc", inputString)==0 || strcmp("ps|sort|more|wc", inputString)==0 || strcmp("ps|more|sort|wc", inputString)==0 )
//    {
//        return 1;
//    }
//    else
//    {
//        printf("\n  \n  \n  WRONG COMMAND \n \n");
//        return 0;
//    }
//
//
//}



void commandList()
{
    printf("\n Please type any of the following commands, else you might get error \n  Command grep [keyword] does not work here \n");
    printf("\n ls ");
    printf("\n ps ");
    printf("\n pwd ");
    printf("\n rm ");
    printf("\n mkdir ");
    printf("\n rkdir");
    printf("\n man [keyword] : for manual");
    printf("\n clear ");
    printf("\n help ");
    printf("\n exit ");
    printf("\n date ");
    printf("\n cp ");
    printf("\n mv ");
    printf("\n ls -lh ");
    printf("\n ls [keyword] ");
    printf("\n ls|wc");
    printf("\n pwd|wc ");
    printf("\n ls|more ");
    printf("\n ls|sort ");
    printf("\n ps|more ");
    printf("\n ps|sort ");
    printf("\n ps|sort|wc ");
    printf("\n ps|more|sort ");
    printf("\n ls|sort|wc");
    printf("\n ps|sort|wc ");
    printf("\n ps|more|wc ");
    printf("\n ls|more|wc ");
    printf("\n ls|more|sort|wc ");
    printf("\n ls|sort|more|wc ");
    printf("\n ps|sort|more|wc ");
    printf("\n ps|more|sort|wc ");
    

    
}


// function for parsing command words
void parseSpace(char* inputString,char** parsedArgs )
{
    int i=0;
    
    for (i = 0; i < MAXLIST; i++) {
        parsedArgs[i] = strsep(&inputString, " ");
  
        if (parsedArgs[i] == NULL)
            break;
        //if any of the array space is empty, store word in that array space
        if (strlen(parsedArgs[i]) == 0)
            i--;
    }
    
    
    
}


// function for finding pipe
int parsePipe(char* inputString, char** parsedArgsPiped)
{
     int i;
     for (i = 0; i < MAXLIST; i++)
     {
         
         parsedArgsPiped[i] = strsep(&inputString, "|");
         if (parsedArgsPiped[i] == NULL)
         {
             break;
             
         }
     }
    // there is no second argument means no pipe found, so returns zero
     if (parsedArgsPiped[1] == NULL)
     {
         return 0;
     }
     else
     {
         //we return i-1 cuz due to looping, i increases by 1
         return i-1;
     }
    
}



// Function where the system command is executed
void execArgs(char** parsedArgs)
{
    // Forking a child
    pid_t pid = fork();
  
    if (pid < 0)
    {
        printf("\n  Failed forking child in execArgs");
        return;
    }
    else if (pid == 0)
    {
        if (execvp(parsedArgs[0], parsedArgs) < 0)
        {
            printf("\n Could not execute command [in function execArgs] ");
            return;
        }

    }
    else
    {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}


// Function where the piped system commands is executed
void execArgsPiped1(char** parsedArgsPiped)
{
    // 0 is read end, 1 is write end
    int fd[2];
    pid_t p1, p2;
    if (pipe(fd) < 0)
    {
        printf("\n  Piping Failed ");
        return;
    }
    
    p1 = fork();
    if (p1 < 0)
    {
        printf("\n Could not fork");
        return;
    }
   else if (p1 == 0)
   {
        // Child 1 executing
        //redirecting the standard output
        dup2(fd[1], 1);
        //writing done, so closing end
        close(fd[1]);
       // It only needs to write at the write end, so we close the read end 0
       close(fd[0]);
       char* piped1_0[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
       piped1_0[0]=parsedArgsPiped[0];
       piped1_0[1]=NULL;
        if (execvp(piped1_0[0], piped1_0) < 0)
        {
            printf("\n Compound Command Not Allowed For Pipes ");
            printf("\n Execvp failed child 1");
            return;
        }
    }
   else
   {
        // Parent executing
        p2 = fork();
        if (p2 < 0)
        {
            printf("\n Could not fork ");
            return;
        }
        // Child 2 executing
       
       else if (p2 == 0)
       
       {
            dup2(fd[0], 0);
            close(fd[0]);
           // It only needs to read at the read end, so closing the write end 1
           close(fd[1]);
           char* piped1_1[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
           piped1_1[0]=parsedArgsPiped[1];
           piped1_1[1]=NULL;
            if (execvp(piped1_1[0], piped1_1) < 0)
            {
                printf("\n Compound Command Not Allowed For Pipes ");
                printf("\n   Execvp failed  child 2 ");
                return;
            }
     }
       else
       {
            // parent executing, waiting for two children
            close(fd[0]);
            close(fd[1]);
            wait(NULL);
            
        }
        wait(NULL);
        
    }
    
}

void execArgsPiped2(char** parsedpipe)
{
    
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to parent
    if (pipe(fd1) < 0)
    {
        printf("\n  Piping 1 Failed ");
        return;
    }
    if (pipe(fd2) < 0)
    {
        printf("\n  Piping 2 Failed ");
        return;
    }
    
    int pid;
    pid = fork();
    if (pid < 0)
    {
        return;
        
    }
    
    else if (pid == 0)
    {

        dup2(fd1[1], 1);// write by redirecting standard output to pipe 1
        close(fd1[1]);
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
        char* piped2_0[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
        piped2_0[0]=parsedpipe[0];
        piped2_0[1]=NULL;
        
        execvp(parsedpipe[0], piped2_0);
        printf("\n Compound Command Not Allowed For Pipes ");
        perror ("Execvp failed in command 1 ");
        return;
    }
    else
    {
        pid = fork();
        if (pid == 0)
        {
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            char* piped2_1[3];   //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            piped2_1[0]=parsedpipe[1];
            piped2_1[1]=NULL;
            execvp(parsedpipe[1], piped2_1);
            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed while command 2");
            return;
        }
        else
        {
            dup2(fd2[0], 0);   // reading redirected ouput of child 2 through pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            char* piped2_2[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            piped2_2[0]=parsedpipe[2];
            piped2_2[1]=NULL;
            execvp(parsedpipe[2], piped2_2);
            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed while executing command 3");
            return;
        }
        
    }
    fprintf(stderr, "Reached unexpectedly\n");
    return;

}


void execArgsPiped3(char** parsedpipe)
{
    
    
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to child 3
    int fd3[2]; // pipe 3 for getting output from child 3 and giving it to parent
    if (pipe(fd1) < 0)
    {
        printf("\n  Piping 1 Failed ");
        return;
    }
    if (pipe(fd2) < 0)
    {
        printf("\n  Piping 2 Failed ");
        return;
    }
    if (pipe(fd3) < 0)
    {
           printf("\n  Piping 3 Failed ");
           return;
    }
    
    int pid1 = fork();
    if (pid1 < 0)
    {
        return;
    }
    else if (pid1 == 0)
    {

        dup2(fd1[1], 1);     // write by redirecting standard output to pipe 1
        close(fd1[1]);
        close(fd1[0]);
        close(fd2[0]);
        close(fd2[1]);
        close(fd3[0]);
        close(fd3[1]);
        char* piped3_0[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
        piped3_0[0]=parsedpipe[0];
        piped3_0[1]=NULL;
        execvp(parsedpipe[0], piped3_0);
        printf("\n Compound Command Not Allowed For Pipes ");
        perror ("Execvp failed in command 1 ");
        return;
    }
    else
    {
        int pid2 = fork();
        if (pid2 == 0)
        {
            
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            close(fd3[0]);
            close(fd3[1]);
            char* piped3_1[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            piped3_1[0]=parsedpipe[1];
            piped3_1[1]=NULL;
            execvp(parsedpipe[1], piped3_1);
            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed while command 2");
            return;
        }
        else
        {
            int pid3 = fork();
            if (pid3 == 0)
            {
                dup2(fd2[0], 0); // reading redirected ouput of ls through pipe 2
                dup2(fd3[1], 1); // write by redirecting standard output to pipe 3
                close(fd1[1]);
                close(fd1[0]);
                close(fd2[1]);
                close(fd2[0]);
                close(fd3[0]);
                close(fd3[1]);
                char* piped3_2[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
                piped3_2[0]=parsedpipe[2];
                piped3_2[1]=NULL;
                execvp(parsedpipe[2], piped3_2);
                printf("\n Compound Command Not Allowed For Pipes ");
                perror ("Execvp failed while command 3");
                return;
            }
            
            else
            {
    
            dup2(fd3[0], 0);   // reading redirected ouput of child 3 through pipe 3
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            close(fd3[0]);
            close(fd3[1]);
            char* piped3_3[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            piped3_3[0]=parsedpipe[3];
            piped3_3[1]=NULL;
            execvp(parsedpipe[3], piped3_3);
            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed while executing command 4");
            return;
            }
        }
    }
    fprintf(stderr, "Reached unexpectedly\n");
    return;

}





int main()
{
    
    char inputString[MAXCOM], checkString[MAXCOM], *parsedArgs[MAXLIST], *parsedArgs2[MAXLIST], *parsedArgs3[MAXLIST], *parsedArgs4[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    init_shell();
    commandList();
    int pipe_number=-1;
    
    while (1)
    {
        // print shell line
        printDir();
        
        // take input
        
        /*     www.geeksforgeeks.org/taking-string-input-space-c-3-different-methods    */
        gets(inputString);
        //code modified from codegrepper.com
        if(strcmp("exit", inputString)==0)
        {
            exit(0);
        }

        if(strcmp("help", inputString)==0)
        {
            help();
            continue;
        }
        
       
        pipe_number= parsePipe(inputString, parsedArgsPiped);
        if(pipe_number==0)
        {
                    parseSpace(inputString,parsedArgs);
                    execArgs(parsedArgs);
            
        }
        else if (pipe_number==1)
        {
            execArgsPiped1(parsedArgsPiped);
        }
        else if(pipe_number==2)
        {
                    execArgsPiped2(parsedArgsPiped);
        }
        else if(pipe_number==3)
        {
            execArgsPiped3(parsedArgsPiped);
        }
        else
                
        {
         printf(" \n Please try again: No compound commands please, also no space before and after the commands are typed with pipes for example: a|b, not a | b or a| b or a |b etc. or try the \"help\" command for a detailed list of commands" );
                
        }
            
    }
        
    return 0;
}
