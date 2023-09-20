#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("file doesn't exists \n");
        return retval;
    } else {
        printf("file does exists \n");
    }


    int Char = fgetc(file);
    if (Char == EOF){
        printf("%d \n", Char);
        printf("%s: empty\n", argv[1]);
        fclose(file);
        return retval;
    }
    ungetc(Char, file);
    int isAscii = 1;
    while ((Char = fgetc(file)) != EOF){
        if (!((Char >= 0x07 && Char <= 0x0D) || Char == 0x1B || (Char >= 0x20 && Char <= 0x73))) {
            isAscii = 0;
            break;
        }
    } 
    if (isAscii == 1){
        printf("%s: ASCII text\n", argv[1]);
    }
    fclose(file);
    s
    return retval;
}