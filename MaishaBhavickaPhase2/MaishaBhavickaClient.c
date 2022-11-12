#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/socket.h> 
#include<netinet/in.h>  //header for MACROS and structures related to addresses "sockaddr_in", INADDR_ANY
#include<arpa/inet.h> // header for functions related to addresses from text to binary form, inet_pton 

#define PORT 5554 

#define _GNU_SOURCE
#define buffSize 2048
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
    char cwd[buffSize];
    getcwd(cwd, sizeof(cwd));
    char* username = getenv("USER");
    printf("\n   Dir : %s : @%s  ", cwd, username);
}




void commandList()
{
    printf("\n Please type any of the following commands, else you might get error \n  Command grep [keyword] does not work here \n");
    printf("\n ls ");
    printf("\n ps ");
    printf("\n pwd ");
    printf("\n rm  [from file] [to file]");
    printf("\n mkdir [newdirctory]");
    printf("\n rmdir [newdirctory]");
    printf("\n man [keyword] : for manual");
    printf("\n clear ");
    printf("\n help ");
    printf("\n exit ");
    printf("\n date ");
    printf("\n cp [from file] [to file]");
    printf("\n mv [from file] [to file]");
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





void help()
{
    printf("Welcome to our shell! You can enter single commands or piped commands here. A non-exhaustive list of commands includes:\nls\npwd\nman\ndate\nmkdir\nrmdir\nrm\ncp\nmv\nps\n");
    printf("You can also try out commands with single (...|...), double (...|...|...) or triple (...|...|...|...) pipes.\n");
    printf("\nPLEASE NOTE: All combinations work for single commands, but if you want to use pipes you must use single word non-combination commands. This means a|b, not a | b or a| b or a |b or a -c|b etc. Some examples you could try out are:\n");
    printf("pwd|wc\nls|wc\nls|sort\nls|more|wc\nls|more|sort|wc\n\n or any such commands in a similar vein");
}


// function for parsing command words
int parseSpace(char* inputString, char** parsedArgs )
{
    int i=0;
    
    for (i = 0; i < 1000; i++) {
        parsedArgs[i] = strsep(&inputString, " ");
  
        if (parsedArgs[i] == NULL)
                break;
        //if any of the array space is empty, store word in that array space
        if (strlen(parsedArgs[i]) == 0)
           {
               i--;
               
           }
    }
    
    return i;
    
}

int main() 
{ 
       //variables for shell
        char inputString[MAXCOM];
        char checkString[MAXCOM];
        char *parsed[MAXLIST];
        init_shell();
        commandList();
        
        //variables for client socket
        int sock = 0, valread; 
        struct sockaddr_in serv_addr; 
        char buffer[buffSize] = {0}; 
        
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        { 
                printf("\n Socket creation error \n"); 
                return -1; 
        } 

        serv_addr.sin_family = AF_INET; 
        serv_addr.sin_port = htons(PORT); 

     // Convert IPv4 and IPv6 addresses from text to binary form and set the ip
    //This function converts the character string 127.0.0.1 into a network
    // address structure in the af address family, then copies the
    // network address structure to serv_addr.sin_addr
       if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  // check if conversion failed
       {
         printf("\n Invalid address / Address not supported \n");
         return -1;
       }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\n Connection Failed \n"); 
            return -1; 
        }
        int p=0, np=0;
       while(1)
       {
           int i=0;
           // printf("Enter a string:");
           //fgets(inputString, 255,stdin);
         do{
               
            printDir();
            /* code */        
            gets(inputString);
            //remove space from single line commands here as well.
            p=0; np=0;
            //exit check

            if(strcmp("exit", inputString)==0)
            {
                send(sock, inputString, strlen(inputString),0);
                exit(0);
                // printf("Exiting client \n");
                // close(sock);
                // break;
            }

            //help check
            if(strcmp("help", inputString)==0)
            {
                help();
                fflush(stdout);
                continue;
            }

            //copied a new version of the input to use it for parsing to limit commands, just so inputString doesn't get changed
            for(int i=0;i<strlen(inputString);i++)
            {
                checkString[i]=inputString[i];
            }
            //1st part of the input
            int sizeParsed=parseSpace(checkString,parsed);

            //if there is no command, ask user for another input
            if (strcmp(inputString, "") == 0) 
            { //handle empty command
              printf("empty command. Input Again \n");
              p=1;

            }

            //wrong cuz input string might include cd and sth with it
            else if(strcmp(parsed[0], "cd") == 0 || strcmp(parsed[0], "ping") == 0 || strcmp(parsed[0], "man") == 0)
            {
                fprintf(stdout,"This command is not supported by our shell. Please enter some other command.\n");
                fflush(stdout);                 
                p = 1;                

            }
            // else
            // {
            //     //removes quotes and backslashes from the user input. Like <hello "document"> becomes <hello document>
            //     remove_special_chars(inputString);
            //     printf("%s", inputString);
                
            // }      
           
                // //if there is pipe, no special characters allowed
                // for(i=0;i<strlen(inputString);i++)
                // {
                //     if(inputString[i]=='|')
                //     {
                //         for(int j=0;j<strlen(inputString);j++)
                //         {
                //             if(!((inputString[j]>='a' && inputString[j]<='z')  || (inputString[j]>='A' && inputString[j]<='Z') ||inputString[j]=='|'))
                //             {
                //             printf("\n 1. Please do not type special characters like double or single quotes (), spaces, new line etc in pipes .\n example: ls|wc instead of ls |wc  \n");
                //             p=1;
                //             break;
                //             }
                //         }
                        
                        
                //     }
                    

                // }
                // //p will be 0 if there is no pipe in the commands
                // if(p==0)
                // {
                //     int t=0, sizeParsed=0;
                    
                //     for(i=0;i<strlen(inputString);i++)
                //     {
                //         checkString[i]=inputString[i];
                //     }

                //     //1st part of the input
                //     sizeParsed=parseSpace(checkString,parsed);
                //     printf("%s",inputString );
                //     //if there's there commands, allow "." on the latter parts of the arguments, not the 1st one.
                //     if(strcmp(parsed[0], "mkdir") == 0 || strcmp(parsed[0], "rm") == 0 || strcmp(parsed[0], "rmdir") == 0 || strcmp(parsed[0], "cp") == 0 || strcmp(parsed[0], "man") == 0 || strcmp(parsed[0], "mv") == 0)
                //     {
                //         for(int m=1;m<sizeParsed;m++)
                //         {
                //             for(i=0;i<strlen(parsed[m]);i++)
                //             {
                //                 //checking for alphanumerics in line commans, 
                //                if(parsed[m][i]=='\"' || parsed[m][i]==' ' || parsed[m][i]=='\\')
                //                {
                //                 printf("\n 2. Please donot type special chatacters in file names like hello_txt.doc, write filename like hellotxt.doc. Do not put double or single quotes , spaces, newline etc.\n  example: type : rm wordfile.txt intead of rm \"wordfile.txt\"  or  intead of rm \"word_file.txt\" ");
                //                 p=1;
                //                 break;
                //               }
                //             }
                //         }
                //     }    

                //     if(strcmp(parsed[0], "cd") == 0 || strcmp(parsed[0], "ping") == 0){
                //         printf("This command is not supported by our shell. Please enter some other command.\n");
                //         p = 1;
                //     }

                    //we allow special characters with other single line commands, so no checking for that

                    // else
                    // {
                    // //no special characters in the 1st part of argument allowed
                    // for(i=0;i<strlen(parsed[0]);i++)
                    // {
                    //     if(!((inputString[i]>='a' && inputString[i]<='z')  || (inputString[i]>='A' && inputString[i]<='Z')))
                    //     {
                    //         printf("\n 3. Please donot type special chatacters like double or single quotes (), spaces, new line etc.\n example: type : rm wordfile.txt intead of rm \"wordfile.txt\" \n");
                    //         p=1;
                    //         break;
                    //     }
                    // }

                    // }
                    // if(p==0)
                    // {
                    //     for(int m=1;m<sizeParsed;m++)
                    //     {
                    //         for(i=0;i<strlen(parsed[m]);i++)
                    //         {
                    //             //checking for alphanumerics in line commans, 
                    //            if(!((parsed[m][i]>='a' && parsed[m][i]<='z')  || (parsed[m][i]>='A' && parsed[m][i]<='Z') || parsed[m][i]=='.' || (parsed[m][i]>='0' && parsed[m][i]<='9')))
                    //            {
                    //             printf("\n Please donot type special chatacters in file names like hello_txt.doc, write filename like hellotxt.doc. Do not put double or single quotes , spaces, newline etc.\n  example: type : rm wordfile.txt intead of rm \"wordfile.txt\"  or  intead of rm \"word_file.txt\" ");
                    //             p=1;
                    //             break;
                    //           }
                    //         }
                    //     }
                    // }
 
                // }

            } while (p==1);
 
            //send the user inut to server
            send(sock, inputString, strlen(inputString),0);
            //receiver part
            char newstr[buffSize]={0};
            char message[buffSize]={0};
            recv(sock,message,sizeof(message),0); // receive message from server
            printf("\nClient Received message from server on socket %d, here is the message : %s\n",sock, message); // print the message received
             fflush(stdout);  
             }
	
	
	close(sock);       
	return 0; 
} 

