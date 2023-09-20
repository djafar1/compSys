#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include<unistd.h>

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("cannot determine (No such file or directory)\n");
        return retval;
    }
    else{
        printf("file exist");
    }

    if ((access(argv[1],R_OK)) == 0){
        printf("cannot determine (Permission denied)\n");
        return retval;
    } 


    int Char = fgetc(file);
    if (Char == EOF){
        printf("%d \n", Char);
        printf("empty\n");
        fclose(file);
        return retval;
    }
    ungetc(Char, file);
    
    while ((Char = fgetc(file)) != EOF){
        if (Char < 0 || 127 < Char){
            printf("This is not an ascii file");
            fclose(file);
            retval = EXIT_FAILURE;
            return retval;
        }
    } 
    printf("ascii\n");
     
    fclose(file);
    return retval;
}

