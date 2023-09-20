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

    if ((access(argv[1],R_OK)) == -1){
        printf("%s: cannot determine (Permission denied)\n", argv[1]);
        return retval;
    } 

    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("%s: cannot determine (No such file or directory)\n", argv[1]);
        return retval;
    }
    else{
        printf("file exist \n");
    }    


    int Char = fgetc(file);
    if (Char == EOF){
        // printf("%d \n", Char);
        printf("%s: empty\n",argv[1]);
        fclose(file);
        return retval;
    }
    ungetc(Char, file);
    int isAscii = 1;
    while ((Char = fgetc(file)) != EOF){

        if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B || (Char >= 0x20 && Char <= 0x7E))) {
            isAscii = 0;
            break;
        }
    } 
    if (isAscii == 1){
        printf("%s: ASCII text\n", argv[1]);
    }

    int isISO8851 = 1;
    while ((Char = fgetc(file)) != EOF){

        if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B ||(Char >= 0x20 && Char <= 0x7E)||(Char >= 0xA0 && Char <= 0xFF))) {
            isISO8851 = 0;
            break;
        }
    } 
    if (isISO8851 == 1){
        printf("%s: ISO-8851 text\n", argv[1]);
    }

    fclose(file);
    
    return retval;
}

