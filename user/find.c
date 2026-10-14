#include "kernel/types.h"          //gives program xv6 basic types
#include "kernel/stat.h"           //gives file information and file types
#include "user/user.h"             //gives declarations for xv6 user functions and system calls
#include "kernel/fs.h"             //gives us xv6 file system structures
//search through directories for files or directories whose names match


int match(char *name, char *pattern);  //figures out if the file name matches what the user is looking for
void find(char *path, char *pattern, int type); //find where the files are that match what the user is looking for





int main(int argc, char *argv[]){  //argv stores in command line arguments
    if(argc !=3 && argc !=5){  //if find doesnt have a directory and pattern & doesnt have 3 or 5 command line arguments
        fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n"); //shows correct usage
        exit(1);                    //stop with error
    }
int type = 0; //0 means any type, find either files or directories

if(argc==5){
     if(strcmp(argv[3], "-type") != 0){   // make sure the option says -type
        fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n"); // usage error
        exit(1);   // invalid command
    }
     if(strcmp(argv[4], "f") == 0){       // user wants only files
        type = 1;                        // 1 means file
    }
    else if(strcmp(argv[4], "d") == 0){  // user wants only directories
        type = 2;                        // 2 means directory
    }
    else{
        fprintf(2, "usage: find <dir> <pattern> [-type f|d]\n");
        exit(1);
    }
}
find(argv[1], argv[2], type);   // start searching the given directory
exit(0);                        // program finished successfully

}





// Write the matcher as a small recursive function and test it on its own
int match(char *name, char *pattern){ // required behavior: handles * pattern matching
    if(*pattern == '\0'){          // if we reached the end of the pattern
        return *name == '\0';     // match only if name also reached the end
    }

if(*pattern == '*'){   // if pattern contains a wildcard *
    return match(name, pattern + 1) || (*name != '\0' && match(name + 1, pattern));
}

if(*name == *pattern){              // if the current characters match
    return match(name + 1, pattern + 1); // move forward in both strings
}

return 0;                           // characters did not match
}



void find(char *path, char *pattern, int type){
    int fileDescrip; //integar that operating system uses to identify opened file or directory
    struct stat file, child;//stores information about file (st) or directory (child), defined in kernal/stat.h
    struct dirent directEntry;//contains entry name and number of entry, defined in kernal/ fs.h
    char buf[512], *p;//create char array to hold 511 char plus null terminator and create a pointer to a char as well
//use p to point to the location in buf where the next file name should be written
    // open directory
    if((fileDescrip = open(path, 0)) < 0){//open() is a system call that opens a directory or file (path), 0, means open in read only mode.
        fprintf(2, "find: cannot open %s\n", path);//if opening directory has failed it will return -1 making it <0, 
        exit(1);//fprintf prints message in terminal, 2 as standard error, message will print find:cannot open, %s is placeholder for string, path is file name, 
    }//ends program if true as standard error

    // checking whether xv6 successfully retrieved information about the opened file or directory.
    if(fstat(fileDescrip, &file) < 0){ //fstat() retrieves info about opened file or directoryby taking in file desciptor and address of stat structure where info will be stored
        close(fileDescrip); //if condition is -1 then close opened file or directory
        return;    
    }

    // checks whether path is a directory, if it isnt a directory then close opened file and stop searching
    if(file.type != T_DIR){ //st.type is type of the opened path, T_DIR is a constant representing a directory
        close(fileDescrip);
        return;
    }

    // prevents from writing past end of buf, 
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)){//strlen(path) is number of char in existing path, +1 is space for /, + DIRSIZ is space for a directory entry name, +1 space for string terminator, and sizeof(buf) is total size of the buffer
        close(fileDescrip);
        return;
    }

    // prepare path for entries
    strcpy(buf, path);//copies (directory name)string stored at path into buf 
    p = buf + strlen(buf);//makes p point to null terminator after last char of buf

//checks whether path already ends with / at of buf is not = /, then at the nullterminator of buf, add /
    if(p > buf && *(p - 1) != '/'){//p>buf means pointer is past beginning of buf AND *(p-1) gets char before p, is not / 
        *p++ = '/';
    }
    // go through directory entries
    //each time loop runs, read() retrieves next directory entry, keep looping while we successfully read one complete directory entry
    while(read(fileDescrip, &directEntry, sizeof(directEntry)) == sizeof(directEntry)){
        if(directEntry.inum == 0){ //skips unused directory entry
            continue;
        }
// skip current and parent directories, checks if the first char is . and the second char is at end of string, then name must be "."
//"." is current directory, ".." is parent directory and if condition skips these to prevent infinite recursion
    if(directEntry.name[0] == '.' && (directEntry.name[1] == '\0' || (directEntry.name[1] == '.' && directEntry.name[2] == '\0'))){
        continue;
    }
    //memmove() copies the directory entry's name (de.name) into the memory location pointed to by p., DIRSIZ is number of bytes to copy
        memmove(p, directEntry.name, DIRSIZ);
        p[DIRSIZ] = '\0';//adds a null terminator at end of copied name area so constructed path is valid C string
 
        if(stat(buf, &child) < 0){ //if there is no information about file name in child, stat(buf, &child) then continue skips the entry
            continue;
        }


        // print if name and requested type match
        if(match(p, pattern) && //calls match function to return bool on whether file name matches user's pattern
        (type == 0 || //user didn't specify a file-type filter, so either type is acceptable.
           (type == 1 && (child.type == T_FILE || child.type == T_DEVICE)) ||//Means the user requested -type f, and the entry is a regular file or device file.
           (type == 2 && child.type == T_DIR))){ //Means the user requested -type d, and the entry is a directory
            printf("%s\n", buf); //if conditions are true, then print file name
           }

        // recursively search subdirectories
        if(child.type == T_DIR){ //checks whether current directory entry is a directory 
            find(buf, pattern, type); //recursive to check inside subdirectory
        }
    }

    close(fileDescrip);
}