



#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<netinet/in.h>  //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include <sys/socket.h> 
#include <arpa/inet.h> 

#define PORT 5554 
#define buffSize 2048
#define _GNU_SOURCE

#define MAX_SIZE 100
#define MAXLIST 100
#define MAXCOM 100

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


//code inspired from https://www.faceprep.in/c/program-to-remove-spaces-from-a-string/
void remove_white_spaces(char *str)
{
    int i = 0, j = 0;
    for (i = 0; i < strlen(str); i ++) 
    {
        if (str[i] != ' ') { 
            str[j++] = str[i];}
        // } else if (str[i+1] == ' ') { 
        //    str[j++] = ' ';
        // } else if (str[i+1] != ' ') { 
        //     str[j++] = ' ';
        // }
    }
    if(j>0)
    {
        str[j]=0;
    }
    // return str;
}


//code ispired from  https://stackoverflow.com/questions/7143878/how-to-remove-quotes-from-a-string-in-c
void remove_special_chars(char *token)
{
    int j = 0;
    for (int i = 0; i < strlen(token); i ++) 
    {
        if (token[i] != '\'' && token[i] != '\"' && token[i] != '\\') { 
            token[j++] = token[i];
        } else if (token[i+1] == '\'' && token[i+i] != '\"' && token[i] == '\\') { 
            token[j++] = '\'';
        } else if (token[i+1] != '\'' && token[i+i] != '\"' && token[i] == '\\') { 
            token[j++] = '\\';
        }
    }
    if(j>0)
    {
        token[j]=0;
    }
}



// function for parsing command words
void parseSpace(char* inputString,char** parsedArgs )
{
    int i=0;
    //removes special characters from input
    remove_special_chars(inputString);

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
    //removes spaces from input
    // remove_white_spaces(inputString);


    //removes special characters from input
    // remove_special_chars(inputString);

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
void execArgs(char** parsedArgs, int newsocket)
{
    // Forking a child
    pid_t pid = fork();
  
    if (pid < 0)
    {
       perror("\n  Failed forking child in execArgs");
        return;
    }
    else if (pid == 0)
    {
        dup2(newsocket, 1);
        char execmsg[buffSize] = {0};
        if(strcmp(parsedArgs[0], "mkdir") == 0 || strcmp(parsedArgs[0], "rm") == 0 || strcmp(parsedArgs[0], "rmdir") == 0 || strcmp(parsedArgs[0], "cp") == 0 || strcmp(parsedArgs[0], "mv") == 0 || strcmp(parsedArgs[0], "cat") == 0)
        {
            int j = snprintf(execmsg, buffSize, "Something has been created or removed using server using socket : %d !", newsocket); // puts string into buffer
            send(newsocket,execmsg,strlen(execmsg),0); // send message to client
            //This function is used to set all the socket structures with null values
            bzero(execmsg, buffSize);   
        }
        close(newsocket);
        if (execvp(parsedArgs[0], parsedArgs) < 0)
        {
            perror("\n Could not execute command [in function execArgs] ");
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
void execArgsPiped1(char** parsedArgsPiped, int newsocket)
{
    // 0 is read end, 1 is write end
    int fd[2];
    pid_t p1, p2;
    if (pipe(fd) < 0)
    {
        perror("\n  Piping Failed ");
        return;
    }
    
    p1 = fork();
    if (p1 < 0)
    {
        perror("\n Could not fork");
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
            perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 1");
            return;
        }
    }
   else
   {
        // Parent executing
        p2 = fork();
        if (p2 < 0)
        {
            perror("\n Could not fork ");
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
           //redirecting standard output of the last end to client
           dup2(newsocket, 1);
           close(newsocket);
            if (execvp(piped1_1[0], piped1_1) < 0)
            {
                perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 2");
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

void execArgsPiped2(char** parsedpipe, int newsocket)
{
    
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to parent
    if (pipe(fd1) < 0)
    {
        perror("\n  Piping 1 Failed ");
        return;
    }
    if (pipe(fd2) < 0)
    {
        perror("\n  Piping 2 Failed ");
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
        perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 1");
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
            perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 2");
            return;
        }
        else
        {
            pid = fork();
            if (pid == 0)
            {
            dup2(fd2[0], 0);   // reading redirected ouput of child 2 through pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            char* piped2_2[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            piped2_2[0]=parsedpipe[2];
            piped2_2[1]=NULL;

            //redirecting standard output of the last end to client
           dup2(newsocket, 1);
           close(newsocket);

            execvp(parsedpipe[2], piped2_2);
            perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 3");
            return;

            }
            else
            {
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            
            wait(NULL);

            }

            wait(NULL);
            
        }

        wait(NULL);
        
        
    }
    return;

}


void execArgsPiped3(char** parsedpipe, int newsocket)
{
    
    
    int fd1[2]; // pipe 1 for getting output from child 1 and giving it to child 2
    int fd2[2]; // pipe 2 for getting output from child 2 and giving it to child 3
    int fd3[2]; // pipe 3 for getting output from child 3 and giving it to parent
    if (pipe(fd1) < 0)
    {
        perror("\n  Piping 1 Failed ");
        return;
    }
    if (pipe(fd2) < 0)
    {
        perror("\n  Piping 2 Failed ");
        return;
    }
    if (pipe(fd3) < 0)
    {
           perror("\n  Piping 3 Failed ");
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
        perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 1");
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
            perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 2");
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
                perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 3");
                return;
            }
            
            else
            {
                int pid4=fork();
                if(pid4==0)
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

                    //redirecting standard output of the last end to client
                    dup2(newsocket, 1);
                    close(newsocket); 
                    execvp(parsedpipe[3], piped3_3);
                    perror("\n Compound Command Not Allowed For Pipes \n Execvp failed while executing command 4");
                    return;
                }
                else
                {
                    close(fd1[1]);
                    close(fd1[0]);
                    close(fd2[1]);
                    close(fd2[0]);
                    close(fd3[0]);
                    close(fd3[1]);
                    
                    wait(NULL);
                }
                 wait(NULL);
    
            
            }
             wait(NULL);
        }
         wait(NULL);
         
    }
   
    return;

}




//int main(int argc, char const *argv[]) 
int main() 
{ 
        int server_fd, new_socket; 
        struct sockaddr_in address; 
        int valread=0;
        int addrlen = sizeof(address); 
        char buffer[buffSize] = {0}; 
        char inputString[255];

        //variables for shell
        char checkString[MAXCOM], *parsedArgs[MAXLIST], *parsedArgs2[MAXLIST], *parsedArgs3[MAXLIST], *parsedArgs4[MAXLIST];
        char* parsedArgsPiped[MAXLIST];
        int execFlag = 0;
        int pipe_number=-1;
        //// Creating socket file descriptor with communication: domain of internet protocol version 4, type of SOCK_STREAM for TCP socket, protocol of the internet  
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
        { 
                perror("socket failed"); 
                exit(EXIT_FAILURE); 
        } 

        
        address.sin_family = AF_INET; 
        address.sin_addr.s_addr = INADDR_ANY; 
        address.sin_port = htons( PORT ); 

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
        { 
                perror("bind failed"); 
                exit(EXIT_FAILURE); 
        } 

        if (listen(server_fd, 10) < 0) 
        { 
                perror("listen"); 
                exit(EXIT_FAILURE); 
        } 
    //   char message[buffSize]={0};
      
      while(1)  // to keep server alive for infintiy
      {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
        { 
            perror("accept"); 
            exit(EXIT_FAILURE); 
        } 
        printf("\n Accepted New Socket, Waiting for New Client \n"); // print the message received  
        while(1) //to keep server-current client communication active
        {
           //allocating the memory dynamically
            char * message = (char*)malloc(buffSize*sizeof(char)); 
            //This function is used to set all the socket structures with null values
            bzero(message, buffSize);   
            valread = read(new_socket , message, buffSize);
            // printf("valread: %d:", valread);
            printf("\n Server Received message from client on socket %d, here is the message : %s\n",new_socket, message); // print the message received  
            
        
        //executing the shell code with the received string
        //code modified from codegrepper.com
        if (strcmp(message, "exit") == 0){ //handle exit command
              printf("Server terminating\n");
              close(new_socket);
              break;
            }
            //dup2(new_socket, STDOUT_FILENO); 
            dup2(new_socket, STDERR_FILENO);
            // close(new_socket);

            // //if there is pipe, no special characters or spaces allowed
            for(int i=0;i<strlen(message);i++)
            {
                if(message[i]=='|')
                {
                    //removes spaces from input
                    remove_white_spaces(message);
                    //removes special characters from input
                    remove_special_chars(message);

                }
            }
                
            pipe_number= parsePipe(message, parsedArgsPiped);
            if(pipe_number==0)
            {
                parseSpace(message,parsedArgs);
                execArgs(parsedArgs, new_socket);
                }
                else if (pipe_number==1)
                {
                    execArgsPiped1(parsedArgsPiped, new_socket);
                }
                else if(pipe_number==2)
                {
                    execArgsPiped2(parsedArgsPiped, new_socket);
                }
                else if(pipe_number==3)
                {
            execArgsPiped3(parsedArgsPiped, new_socket);
            }
            else
            {
                printf(" \n Please try again: No compound commands please, also no space before and after the commands are typed with pipes for example: a|b, not a | b or a| b or a |b etc. or try the \"help\" command for a detailed list of commands " );
            }
            free(message);
            fflush(stdout);
           
        
        }
        close(new_socket);	
	
    }
	close(server_fd);
    return 0; 

} 
