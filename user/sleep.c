#include "kernel/types.h"         //gives program xv6's basic types
#include "user/user.h"            //declarations for xv6 user functions and system calls

int main(int argc, char *argv[]){   //argv stores the command line pieces
    if(argc != 2){                  //2 command line pieces for correct command
        fprintf(2, "usage: sleep <ticks>\n"); //2 sends error message to stderr
        exit(1);                     //leave program
    }

for(int i=0; argv[1][i] != '\0'; i++){         //go through each character in ticks
    if(argv[1][i] < '0' || argv[1][i] > '9'){  //check if character is not a digit
        fprintf(2, "usage: sleep <ticks>\n");  //print error to stderr
        exit(1);                               //exit because input is invalid
    }
}

int ticks = atoi(argv[1]);             //converts ticks from a string to an integer
pause(ticks);                          //pause for the given number of ticks
exit(0);                               //leave program 


}