#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <unistd.h>


int print_error(char *path, int errnum) {
    return fprintf(stdout, "%s: cannot determine (%s)\n",
    path, strerror(errnum));
}

enum file_type{
    DATA,
    EMPTY,
    ASCII,
    ISO8859,
    UTF8
};

const char * const FILE_TYPE_STRINGS[] = {
    "data",
    "empty",
    "ASCII text",
    "ISO-8859 text",
    "Unicode text, UTF-8 text"
};

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    //No file given
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }
    
    FILE *file = fopen(argv[1], "r");   


    if (access(argv[1], F_OK) == -1) {
        print_error(argv[1],2);
        return retval;
    } 

    // Check if the file is readable
    if (access(argv[1], R_OK) == -1) {
        print_error(argv[1],13);
        return retval;
    }
    

    enum file_type type = DATA;

    //File type Empty
    int Char = fgetc(file);
    if (Char == EOF){
        type = EMPTY;
        //fclose(file) vi lukker den tilsidst
        //return retval vi lukker den tilsidst og return retval er kun for fejl.
    }

    //File type ASCII
    ungetc(Char, file); 
    while ((Char = fgetc(file)) != EOF){
        if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B || (Char >= 0x20 && Char <= 0x7E))) {
            ungetc(Char, file);
            type = DATA;
            break;
        }
        else{
            type = ASCII;
        }
    }

    //File type ISO-8851
    while ((Char = fgetc(file)) != EOF){
        if (!((Char >= 0x07 && Char <= 0x0D) ||(Char == 0x1B)||(Char >= 0x20 && Char <= 0x7E)||(Char >= 0x7f && Char <= 0xFF))) {
            type = DATA;
            break;
        }
        else{
            type = ISO8859;
        }
    }
     

    //File type UTF-8
    // while ((Char = fgetc(file)) != EOF){
    //     if (!!((Char >= 0x20) ||Char == 0x0A|| Char == 0x0D|| Char == 0x1B ||
    //     (Char >= 0x20 && Char <= 0x7E)||(Char >= 0x7f && Char <= 0x10FFFF))) {
    //         break;
    //     }
    //     else{
    //         type = UTF8;
    //         break; 
    //     }
    // }

    fclose(file);
    printf("%s: %s\n", argv[1], FILE_TYPE_STRINGS[type]);


    return retval;
}

