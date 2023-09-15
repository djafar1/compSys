#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int main(int argc, char* argv[]) {
    if (argc != 2)
        return 1;
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("file doesn't exists \n");
    } else {
        printf("file does exists \n");
    }

    int Char = fgetc(file);
    if (Char == EOF){
        printf("%d \n", Char);
        printf("empty\n");
        fclose(file);
        return 0;
    }
    ungetc(Char, file);
    
    while ((Char = fgetc(file)) != EOF){
        if (Char < 0 || 127 < Char){
            printf("This is not an ascii file");
            fclose(file);
            return 1;
        }
    } 
    printf("ascii\n");
     
    fclose(file);
    return 0;
}

