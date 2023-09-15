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
    int c = fgetc(file);
    if (c == EOF){
        printf("%d \n", c);
        printf("empty\n");
        fclose(file);
        return 0;
    }
    printf("%d \n", c);
    putchar(c);
    putchar(50);
    fclose(file);
    return 0;
}
