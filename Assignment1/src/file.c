#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <unistd.h>

<<<<<<< HEAD
typedef enum FileType{
=======

int print_error(char *path, int errnum) {
    return fprintf(stdout, "%s: cannot determine (%s)\n",
    path, strerror(errnum));
}

enum file_type{
    DATA,
>>>>>>> 8d4e92d6cb24c4153661d2bf81943844f76716dc
    EMPTY,
    DATA,
    ASCII,
    ISO8859,
    UTF8,
} FileType;


const char * const FILE_TYPE_STRINGS[] = {
    "empty",
    "data",
    "ASCII text",
    "ISO-8859 text",
    "Unicode text, UTF-8 text"
};

int check_ASCII(const unsigned char byte){
    if ((byte >= 0x07 && byte <= 0x0D) || (byte == 0x1B) ||
           (byte >= 0x20 && byte <= 0x7E)){
            return 1;
    }
    return 0;

    
}

int check_ISO_8859_1(const unsigned char byte) {
    if (byte >= 128){
        return 1;
    }
    return 0;
}


int utf8_sequence_length(const unsigned char byte)
{   
    if ((byte & 0xC0) == 0xC0)
    {
        return 1;
    }
    else if ((byte & 0xE0) == 0xE0)
    {
        return 2;
    }
    else if ((byte & 0xF0) == 0xF0)
    {
        return 3;
    }
    return 0;
}

int check_UTF8(const unsigned char byte, FILE *const file)
{
    int sequence_length = utf8_sequence_length(byte);
    if (sequence_length == 0){
        return 0;
    }
    long original_pos = ftell(file);
    unsigned char next_byte;
    for (int i = 0; i < sequence_length; ++i)
    {
        if (fread(&next_byte, sizeof(char), 1, file) == 0 ||
            (next_byte ^ 0x80) >> 6 != 0x00)
        {
            fseek(file, original_pos, SEEK_SET);
            return 0;
        }
    }
    return 1;
}

FileType get_file_type(FILE *fp){
    unsigned char byte;
    size_t size = sizeof(unsigned char);
    if (fread(&byte, size, 1, fp) == 0){
        return EMPTY;
    }
    fseek(fp, 0, SEEK_SET); // Start from the beginning of the file.
    long original_pos;

    FileType ft = ASCII;
    while (fread(&byte, size, 1, fp) == 1){
        if (check_ASCII(byte)){
            continue; 
        }
        if (check_UTF8(byte, fp)){
            ft = UTF8;
            continue;
        }
        if (check_ISO_8859_1(byte)){
            original_pos = ftell(fp); // Save the current position 
            if (ft == UTF8){
                fseek(fp, original_pos, SEEK_SET); 
            }
            else{
                ft = ISO8859;
                continue;
            }
        }
        else{
            // If all unknown return DATA
            return DATA;
        }
    }
    return ft;
}

int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    //No file given
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }
<<<<<<< HEAD
    // Not readable file
    if ((access(argv[1], R_OK)) == -1){
        printf("%s: cannot determine (Permission denied)\n", argv[1]);
        return retval;
    }
    //File not found
    FILE *file = fopen(argv[1], "r");
    if (file == NULL){
        printf("%s: cannot determine (No such file or directory)\n", argv[1]);
        return retval;
    }
    FileType type = get_file_type(file);
=======
    
    FILE *file = fopen(argv[1], "r");   


    if (access(argv[1], F_OK) == -1) {
        print_error(argv[1],2);
        return retval;
    } 

    // Check if the file is readable
    if (access(argv[1], R_OK) == -1) {
        print_error(argv[1],1);
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

>>>>>>> 8d4e92d6cb24c4153661d2bf81943844f76716dc
    fclose(file);
    printf("%s: %s\n", argv[1], FILE_TYPE_STRINGS[type]);
    return retval;
}