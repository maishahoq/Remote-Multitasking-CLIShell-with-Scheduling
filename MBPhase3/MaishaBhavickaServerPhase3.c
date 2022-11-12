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
#include <signal.h> // header for signal related functions and macros declarations
#include <pthread.h>

#define PORT 5554 
#define buffSize 2048
#define _GNU_SOURCE

#define MAX_SIZE 100
#define MAXLIST 100
#define MAXCOM 100


int checkExit=0;


void* HandleClient(void* new_socket);

// function routine of Signal Handler for SIGINT, to terminate all the threads which will all be terminated as we are calling exit of a process instead of pthread_exit
void serverExitHandler(int sig_num)
{
    printf("\n Exiting server.  \n");
    fflush(stdout);// force to flush any data in buffers to the file descriptor of standard output,, a pretty convinent function
    exit(0);
}

//code inspired from https://www.faceprep.in/c/program-to-remove-spaces-from-a-string/
void remove_white_spaces(char *str)
{
    int i = 0, j = 0;
    for (i = 0; i < strlen(str); i ++) 
    {
        
        if (str[i] != ' ') { 
            str[j++] = str[i];}
        
    }
    if(j>0)
    {
        str[j]=0;
    }
    
}

//to remove leading and trailing white spaces, code taken from codeforwin.org
void trim(char *str){
    int index, i;

    /*
     * Trim leading white spaces
     */
    index = 0;
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n')
    {
        index++;
    }

    /* Shift all trailing characters to its left */
    i = 0;
    while(str[i + index] != '\0')
    {
        str[i] = str[i + index];
        i++;
    }
    str[i] = '\0'; // Terminate string with NULL


    /*
     * Trim trailing white spaces
     */
    i = 0;
    index = -1;
    while(str[i] != '\0')
    {
        if(str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
        {
            index = i;
        }

        i++;
    }

    /* Mark the next character to last non white space character as NULL */
    str[index + 1] = '\0';
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
        printf("\n  Failed forking child in execArgs");
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
void execArgsPiped1(char** parsedArgsPiped, int newsocket)
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

        //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
         
        char* piped1_0[20]={0};//we set all the elems to 0 so we can check for how many string elems we have in the array
        parseSpace(parsedArgsPiped[0],piped1_0); //parse the first element of the parsed pipe array, to support compound commands
        // //printf("Reaching here\n");
        // //printf("%s, %s\n", piped1_0[0], piped1_0[1]);
        int y=0;
        int length1=0;
        while(piped1_0[length1]!=0)
        {
            length1++;

        }
        
        //setting the first non-character empty space to NULL
        for(int y=length1;y<20;y++)
        {
            piped1_0[y]=NULL;

        }
        
      
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
            char* piped1_1[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
            parseSpace(parsedArgsPiped[1],piped1_1); //parse the first element of the parsed pipe array, to support compound commands; similar process for other piped commands too
            int length2=0;
            while(piped1_1[length2]!=0)
            {
            length2++;

            }
        //piped1_1[2]=NULL;
        for(int y=length2;y<20;y++)
        {
            piped1_1[y]=NULL;

        }
           //redirecting standard output of the last end to client
           dup2(newsocket, 1);
           close(newsocket);
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
            //wait(NULL);
            
        }
        //wait(NULL);
       
    }
    
}

void execArgsPiped2(char** parsedpipe, int newsocket)
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

        char* piped2_0[20]={0};//we set all the elems to 0 so we can check for how many string elems we have in the array
        parseSpace(parsedpipe[0],piped2_0);
        //printf("Reaching here\n");
        //printf("%s, %s\n", piped1_0[0], piped1_0[1]);
        int y=0;
        int length1=0;
        while(piped2_0[length1]!=0)
        {
            length1++;

        }
        
        for(int y=length1;y<20;y++)
        {
            piped2_0[y]=NULL;

        }

        if(execvp(piped2_0[0], piped2_0) < 0){

            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed in command 1 ");
            return;
        }
        
    }
    else
    {
        pid = fork();

        if (pid < 0)
        {
            printf("\n Could not fork ");
            return;
        }

        else if (pid == 0)
        {
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            // char* piped2_1[3];   //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            // piped2_1[0]=parsedpipe[1];
            // piped2_1[1]=NULL;

            char* piped2_1[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
            parseSpace(parsedpipe[1],piped2_1);
            int length2=0;
            while(piped2_1[length2]!=0)
            {
                length2++;

            }
            //piped1_1[2]=NULL;
            for(int y=length2;y<20;y++)
            {
                piped2_1[y]=NULL;

            }


            if(execvp(piped2_1[0], piped2_1)<0){
                printf("\n Compound Command Not Allowed For Pipes ");
                perror ("Execvp failed while command 2");
                return;
            }
            
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
            // char* piped2_2[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            // piped2_2[0]=parsedpipe[2];
            // piped2_2[1]=NULL;

            char* piped2_2[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
            parseSpace(parsedpipe[2],piped2_2);
            int length3=0;
            while(piped2_2[length3]!=0)
            {
                length3++;

            }
        //piped1_1[2]=NULL;
        for(int y=length3;y<20;y++)
        {
            piped2_2[y]=NULL;

        }

            //redirecting standard output of the last end to client
           dup2(newsocket, 1);
           close(newsocket);

            if(execvp(piped2_2[0], piped2_2) < 0){
                printf("\n Compound Command Not Allowed For Pipes ");
                perror ("Execvp failed while executing command 3");
                return;
            }
            

            }
            else
            {
                close(fd1[1]);
                close(fd1[0]);
                close(fd2[1]);
                close(fd2[0]);
                
            //wait(NULL);

            }

            //wait(NULL);
            
        }

        //wait(NULL);
        
        
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
        printf("\n Could not fork");
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
        // char* piped3_0[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
        // piped3_0[0]=parsedpipe[0];
        // piped3_0[1]=NULL;

        char* piped3_0[20]={0};//we set all the elems to 0 so we can check for how many string elems we have in the array
        parseSpace(parsedpipe[0], piped3_0);
        //printf("Reaching here\n");
        //printf("%s, %s\n", piped1_0[0], piped1_0[1]);
        int y=0;
        int length1=0;
        while(piped3_0[length1]!=0)
        {
            length1++;

        }
        
        for(int y=length1;y<20;y++)
        {
            piped3_0[y]=NULL;

        }

        if(execvp(parsedpipe[0], piped3_0)<0)
        {
            printf("\n Compound Command Not Allowed For Pipes ");
            perror ("Execvp failed in command 1 ");
            return;
        }
        
    }
    else
    {
        int pid2 = fork();

        if (pid2 < 0)
        {
            printf("\n Could not fork ");
            return;
        }

        else if (pid2 == 0)
        {
            
            dup2(fd1[0], 0); // reading redirected ouput of ls through pipe 1
            dup2(fd2[1], 1); // write by redirecting standard output to pipe 2
            close(fd1[1]);
            close(fd1[0]);
            close(fd2[1]);
            close(fd2[0]);
            close(fd3[0]);
            close(fd3[1]);
            // char* piped3_1[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
            // piped3_1[0]=parsedpipe[1];
            // piped3_1[1]=NULL;


            char* piped3_1[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
            parseSpace(parsedpipe[1],piped3_1);
            int length2=0;
            while(piped3_1[length2]!=0)
            {
                length2++;

            }
            //piped1_1[2]=NULL;
            for(int y=length2;y<20;y++)
            {
                piped3_1[y]=NULL;

            }

            if(execvp(piped3_1[0], piped3_1)<0){
                printf("\n Compound Command Not Allowed For Pipes ");
                perror ("Execvp failed while command 2");
                return;
            }
            
        }
        else
        {
            int pid3 = fork();

            if (pid3 < 0)
            {
                printf("\n Could not fork ");
                return;
            }


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
                // char* piped3_2[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
                // piped3_2[0]=parsedpipe[2];
                // piped3_2[1]=NULL;


                char* piped3_2[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
                parseSpace(parsedpipe[2],piped3_2);
                int length2=0;
                while(piped3_2[length2]!=0)
                {
                    length2++;

                }
        //piped1_1[2]=NULL;
        for(int y=length2;y<20;y++)
        {
            piped3_2[y]=NULL;

        }
                if(execvp(piped3_2[0], piped3_2)<0){
                    printf("\n Compound Command Not Allowed For Pipes ");
                    perror ("Execvp failed while command 3");
                    return;
                }
               
            }
            
            else
            {
                int pid4=fork();

                if (pid4 < 0)
                {
                    printf("\n Could not fork ");
                    return;
                }

                if(pid4==0)
                {
                    dup2(fd3[0], 0);   // reading redirected ouput of child 3 through pipe 3
                    close(fd1[1]);
                    close(fd1[0]);
                    close(fd2[1]);
                    close(fd2[0]);
                    close(fd3[0]);
                    close(fd3[1]);
                    // char* piped3_3[3]; //using a seperate array of strings so that I can add NULL at the end of the vector sent to execvp
                    // piped3_3[0]=parsedpipe[3];
                    // piped3_3[1]=NULL;

                    char* piped3_3[20]={0};    //we set all the elems to 0 so we can check for how many string elems we have in the array
                    parseSpace(parsedpipe[3],piped3_3);
                    int length2=0;
                    while(piped3_3[length2]!=0)
                    {
                        length2++;

                    }
                
                    for(int y=length2;y<20;y++)
                    {
                        piped3_3[y]=NULL;

                    }
                    //redirecting standard output of the last end to client
                    dup2(newsocket, 1);
                    close(newsocket); 
                    execvp(piped3_3[0], piped3_3);
                    printf("\n Compound Command Not Allowed For Pipes ");
                    perror ("Execvp failed while executing command 4");
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
                    
                    //wait(NULL);
                }
                 //wait(NULL);
    
            
            }
             //wait(NULL);
        }
         //wait(NULL);
         
    }
   
    return;

}




//int main(int argc, char const *argv[]) 
int main() 
{ 

          //Set the SIGINT (Ctrl-C) signal handler to serverExitHandler 
        signal(SIGINT, serverExitHandler);

        //variables
        int server_fd, new_socket; 
        struct sockaddr_in address; 
        // int valread=0;
        int addrlen = sizeof(address); 
        
        // Creating socket file descriptor with communication: domain of internet protocol version 4, type of SOCK_STREAM for TCP socket, protocol of the internet  
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

        // checkExit=0;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
        { 
            perror("accept failed"); 
            exit(EXIT_FAILURE); 
        } 


        int rc; // return value from pthread_create to check if new thread is created successfukky                           */
        int* thread_socket = (int*)malloc(sizeof(int)); // for passing safely the integer socket to the thread
        if ( thread_socket == NULL ) 
        {
          fprintf(stderr, "Couldn't allocate memory for thread new socket argument.\n");
          exit(EXIT_FAILURE);
        }
        *thread_socket = new_socket;
        //if previous thread exited, i.e flag=1, we enter to create a new thread to execute a new command
        pthread_t  client_id;  // client's ID (just an integer, typedef unsigned long int) to indetify new thread
        // create a new thread that will handle the communication with the newly accepted client
        rc = pthread_create(&client_id, NULL, HandleClient, (void*)thread_socket);  
        if(rc)      // if rc is > 0 imply could not create new thread 
        {
          printf("\n ERROR: return code from pthread_create is %d \n", rc);
          exit(EXIT_FAILURE);
        }
    //pthread_join(thread_id, NULL);
    }
  //we didn't close the client socket new_socket cuz every new client will be handled by a new thread

  //closing the sockets
  close(server_fd);  
  // pthread_exit(NULL);       // terminate the main thread 
  return 0;
}

// Function that handles the client thread
void* HandleClient(void* new_socket)
{

    pthread_detach(pthread_self()); // detach the thread as we don't need to synchronize/join with the other client threads, their execution/code flow does not depend on our termination/completion 
    int socket = *(int*)new_socket;
    if(new_socket!=NULL){free(new_socket);}
    printf("handling new client in a thread using socket: %d\n", socket);
    printf("Listening to client....\n"); // while printing make sure to end your strings with \n or \0 to flush the stream, other wise if in anyother concurent process is reading from socket/pipe-end with standard input/output redirection, it will keep on waiting for stream to end. 
  


        char buffer[buffSize] = {0}; 
        char inputString[255];

        //variables for shell
        char checkString[MAXCOM], *parsedArgs[MAXLIST], *parsedArgs2[MAXLIST], *parsedArgs3[MAXLIST], *parsedArgs4[MAXLIST];
        char* parsedArgsPiped[MAXLIST];
        int execFlag = 0;
        int pipe_number=-1;
        int valread=0;

        checkExit=0;
        printf("\n Accepted New Socket, Waiting for New Client \n"); // print the message received  
         //allocating the memory dynamically
          char * message = (char*)malloc(buffSize*sizeof(char)); 
          pid_t pid = fork();
          if(pid < 0){
            printf("failure in forking inside main \n");
            exit(EXIT_FAILURE);
          }
          else if(pid == 0){ // child process, perform reading from socket here
            // //allocating the memory dynamically
            // char * message = (char*)malloc(buffSize*sizeof(char)); 
            //This function is used to set all the socket structures with null values
            bzero(message, buffSize);   
             // clearing the message arrays properly
            memset(message, 0,buffSize*(sizeof(char))); 
            valread = read(socket , message, buffSize);
             fflush(stdout);// force to flush any data in buffers to the file descriptor of standard output
            // printf("valread: %d:", valread);
            printf("\n Server Received message from client on socket %d, here is the message : %s\n",socket, message); // print the message received  
             
             fflush(stdout);// force to flush any data in buffers to the file descriptor of standard output
        
        //executing the shell code with the received string
        //code modified from codegrepper.com
        if (strcmp(message, "exit") == 0){ //handle exit command
              printf("Server terminating in child\n");
              checkExit=1;
              close(socket);
              //break;
            }
            //redircting the standard output in the child
            dup2(socket, STDOUT_FILENO); 
            dup2(socket, STDERR_FILENO);
            // close(new_socket);

            // //if there is pipe, no special characters or spaces allowed
            for(int i=0;i<strlen(message);i++)
            {
                if(message[i]=='|')
                {
                    //removes spaces from input
                    //remove_white_spaces(message);

                    //removes leading and trailing white spaces from input
                    trim(message);
                    //removes special characters from input
                    remove_special_chars(message);

                }
            }
                
            pipe_number= parsePipe(message, parsedArgsPiped);
            if(pipe_number==0)
            {
                parseSpace(message,parsedArgs);
                execArgs(parsedArgs, socket);
            }
            else if (pipe_number==1)
            {
                execArgsPiped1(parsedArgsPiped, socket);
            }
            else if(pipe_number==2)
            {
                execArgsPiped2(parsedArgsPiped, socket);
            }
            else if(pipe_number==3)
            {
                execArgsPiped3(parsedArgsPiped, socket);
            }
            else
            {
                printf(" \n Please try again: No compound commands please, also no space before and after the commands are typed with pipes for example: a|b, not a | b or a| b or a |b etc. or try the \"help\" command for a detailed list of commands " );
            }
            free(message);
            fflush(stdout);
            exit(EXIT_SUCCESS);
          }
          else{
           //in parent
            wait(NULL);
            if(checkExit==1)
            {
                printf("Server terminating in parent\n");
                close(socket);  
            }
            free(message);
          }       
    close(socket);    
    
    
    // close(server_fd);
     return 0; 
    //pthread_exit(NULL);// terminate the thread

} 
