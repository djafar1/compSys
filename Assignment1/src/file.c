#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <unistd.h>

enum file_type{
    DATA,
    EMPTY,
    ASCII,
    ISO8851,
    UTF8
};

const char * const FILE_TYPE_STRINGS[] = {
    "Data text",
    "empty File",
    "ASCII text",
    "ISO-8852 text",
    "UTF-8 Unicode text"
};

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    //No file given
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }

    //Not readable file
    if ((access(argv[1],R_OK)) == -1){
        printf("%s: cannot determine (Permission denied)\n", argv[1]);
        return retval;
    } 

    //File not found
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("%s: cannot determine (No such file or directory)\n", argv[1]);
        return retval;
    }   

    enum file_type type = DATA;

    //File type Empty
    int Char = fgetc(file);
    if (Char == EOF){
        type = EMPTY;
        fclose(file);
        return retval;
    }

    //File type ASCII
    ungetc(Char, file); 
    while ((Char = fgetc(file)) != EOF){
        if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B || (Char >= 0x20 && Char <= 0x7E))) {
            break;
        }
        else{
            type = ASCII;
            break;
        }
    }

    //File type ISO-8851
    if ((Char = fgetc(file)) != EOF){
        while ((Char = fgetc(file)) != EOF){
            if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B ||
            (Char >= 0x20 && Char <= 0x7E)||(Char >= 0xA0 && Char <= 0xFF))) {
                break;
            }
            else{
                type = ISO8851;
                break;
            }
        }
    } 

    //File type UTF-8
    if ((Char = fgetc(file)) != EOF){
        while ((Char = fgetc(file)) != EOF){
            if (!(Char >= 0x00 && Char <= 0x10FFFF)) {
                break;
            }
            else{
                type = UTF8;
                break;
            }
        }
    } 

    fclose(file);
    printf("%s: %s \n", argv[1], FILE_TYPE_STRINGS[type]);


    return retval;
}

