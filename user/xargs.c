#include "kernel/types.h"   // xv6 basic types
#include "kernel/param.h"   // gives us MAXARG
#include "user/user.h"      // xv6 user functions and system calls
//xargs reads words from standard input and adds them as arguments to a user-specified command
//example: echo a b | xargs echo x
//the pipe(|) sends a b to xargs which runs echo x a b and prints x a b



// prints an error message when user enters invalid command using xargs
//tells program how to correctly use the program
void usage(){
    fprintf(2, "usage: xargs [-n N] cmd [args...]\n"); //prints out (stderr), xargs and [-n N] controls how many input words to use per run, cmd is command to execute and [args...] is optional additional arguments for the command
    exit(1); //exit program with failure
}




// run the command using fork() to make child to run command and wait for it to finish
int runCommand(char *args[]){ //char *args[] represents the command that xargs will execute and all the arguments that will be passsed to that command 
    int pid = fork(); //fork() creates a new process called child process to execute command
                    //char *args[] tells the child which command to execute and what arguments to use
    if(pid < 0){ // Check whether fork failed to create a child process
        return 1; //if failed runCommand returns 1 to indicate failure
    }

//If the process is the child, it executes the requested command, and if execution fails, 
//it prints an error message and exits with status 1.
    if(pid == 0){
        exec(args[0], args);//Run the program named in args[0] with all arguments stored in args[]

        // line 34, and 35 only execute if exec() fails
        fprintf(2, "xargs: exec %s failed\n", args[0]); //print stderr that exec has failed to run command args[0]
        exit(1);
    }

    int status;             //create integer variable to hold child's exit status
    //commands run one at a time for this
    if(wait(&status)< 0){// make parent process wait for child to finish, get the memory address where childs exit status will be stored &status
        return 1;
    }        
    if(status != 0){ //if status isnt zero child has not exited successfully
        return 1;  //returns 1 to indicate failure
    }
    return 0;  //child command finished successfully
}








//read the user's command and standard input, collect the input words, and execute the requested command with those words as arguments.
int main(int argc, char *argv[]){ //takes in number of command line arguments argv and array containing arguments as strings argv
    // xargs needs at least a command
    if(argc < 2){ //if argc is less then 2, user didnt provide command for xargs to execute
        usage(); //print error message and terminate program
    }
    int n = 0;             // 0 means -n was not used, stores maximum number of input words per execution
    int cmdIndex = 1;      //stores index in argv where command to execute begins, which is at index 1
// Checks whether the user entered -n as the first argument after xargs, example: xargs -n 2 echo x, is true
    if(strcmp(argv[1], "-n") == 0){  
// check if enough arguments exist, if user enterss xargs -n 2, argc = 3 and exit with error because no command after 2. 
        if(argc < 4){
            usage();
        }

//This checks whether the string containing N is empty. If it is empty, the program prints the usage message and exits.
        if(argv[2][0] == '\0'){
            usage();
        }

// make sure N contains only digits
        for(int i = 0; argv[2][i] != '\0'; i++){//loops through the string representing n until it hits a nullterminator
            if(argv[2][i] < '0' || argv[2][i] > '9'){ //make sure each element in the string representing N contains digits 0-9,if it doesnt exit
                usage();
            }
        }

        n = atoi(argv[2]);//converts string representing value of n into an integer

//check that n is positive
        if(n < 1){
            usage();
        }

//when -n is present, the command comes after so update command index
        cmdIndex = 3;
    }

//baseArgs is how many arguments were originally supplied for the command
    int baseArgs = argc - cmdIndex;
//checks how many arguments user provided for command and if there is enough room to append input words
    if(baseArgs >= MAXARG){//-n N should not cause the program to construct a command with more than 32 arguments
        usage();
    }
// Maximum number of words that can be appended
    int maxInput = MAXARG - baseArgs;

// With -n, do not allow more words than fit
    if(n > maxInput){
        n = maxInput;
    }

//create an array to hold command name and all arguments passed to that command 
    char *cmdArgs[MAXARG + 1];
//Copies the command name and any arguments originally supplied by the user into cmdArgs
    for(int i = 0; i < baseArgs; i++){
        cmdArgs[i] = argv[cmdIndex + i];
    }

//create variables for reading standard input
    // store words read from stdin
    char buf[512];//store char of input words
    int pos = 0; //index where next char will be stored

    // number of input words currently collected
    int inputCount = 0;

    //create pointer to where each word begins in buf 
    char *inputArgs[MAXARG];
    int result;          // Stores the return value of read()
    int failed = 0;//records whether an error occurred
    char c; //holds one char read from standard input


//reads one char at a time from standard input until there is no more input, 
//0 meand stdin, 1 to read one byte, continue while only one byte was successfully read
    while((result = read(0, &c, 1)) == 1){
//check whether char is part of a word, if it is part of word, then go through if statement
        if(c != ' ' && c != '\t' && c != '\r' && c != '\n'){// 
            if(pos >= sizeof(buf) - 1){ //prevents from writing beyond buffer
                fprintf(2, "xargs: input too long\n");
                failed = 1;
                break;
            }
  //checks whether current char begins new word, buf[pos - 1] == '\0' — The previous word ended
            if(pos == 0 || buf[pos - 1] == '\0'){
                if(inputCount >= maxInput){ //This prevents collecting more input words than can fit alongside the original command arguments
                   fprintf(2, "xargs: too many arguments\n");
                    failed = 1;
                    break;
                }

                inputArgs[inputCount] = &buf[pos];//saves a pointer to the beginning of the new word.
                inputCount++;//increases the number of words collected.
            }

            buf[pos++] = c; //stores the character at buf[pos] and then increments pos
        }

        else {

 //When whitespace is encountered, finish the current word by adding a null terminator.
            if(pos > 0 && buf[pos - 1] != '\0'){
                // Check that there is room for the null terminator
                if(pos >= sizeof(buf)){
                    fprintf(2, "xargs: input too long\n");
                    failed = 1;
                    break;
                }
                buf[pos++] = '\0';// End the current word
            }
//If the user didn't specify -n, execute the command whenever a non-empty input line ends.
            if((n == 0 && c == '\n')||(n > 0 && inputCount == n)){
 // Only execute if words were collected
                if(inputCount > 0){
//The loop appends the input words to the command arguments.
                    for(int i = 0; i < inputCount; i++){
                        cmdArgs[baseArgs + i] = inputArgs[i];
                    }
//mark the end of the argument array
                    cmdArgs[baseArgs + inputCount] = 0;
//execute command and check for failure
                    if(runCommand(cmdArgs) != 0){
                        failed = 1;
                    }
                }
                // Reset for the next line or group
                    pos = 0;
                    inputCount = 0;
             }
    }
}
 // Detect a read error
    if(result < 0){
        fprintf(2, "xargs: input read failed\n");
        failed = 1;
    }
  if(failed){
        exit(1);
    }
    // run any words left over when EOF is reached
    if(inputCount > 0){

        // finish last word if needed
        if(pos > 0 && buf[pos - 1] != '\0'){
            if(pos >= sizeof(buf)){
                fprintf(2, "xargs: input too long\n");
                exit(1);
            }
            buf[pos++] = '\0';
        }

        for(int i = 0; i < inputCount; i++){
            cmdArgs[baseArgs + i] = inputArgs[i];
        }

        cmdArgs[baseArgs + inputCount] = 0;

        if(runCommand(cmdArgs) != 0){
            failed = 1;
        }
    }


    // exit 0 only if every command succeeded
    if(failed){
        exit(1);
    }

    exit(0);
}

