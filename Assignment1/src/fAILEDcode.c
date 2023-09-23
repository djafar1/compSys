#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <unistd.h>

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

int check_ASCII(const unsigned char byte){
    // We consider ASCII to be 0x07-0x0D, 0x1B and 0x20-0x7E
    return (byte >= 0x07 && byte <= 0x0D) || (byte == 0x1B) ||
           (byte >= 0x20 && byte <= 0x7E);
}

// We will only count it as ISO-8859-1 if it is outside the ASCII range.
// This is because ASCII is a subset of ISO-8859-1.
// We do not have to check if byte <= 255, because unsigned char is always <= 255.
int check_ISO_8859_1(const unsigned char byte){ 
    return byte >= 160 && byte <= 255; 
}

#define MASK_HEADER_BYTE2 0xC0 //11000000
#define MASK_HEADER_BYTE3 0xE0 // 1110xxxx
#define MASK_HEADER_BYTE4 0xF0 // 11110xxx
#define MASK_CONTINUATION_BYTE 0x80 // 10xxxxxx

#define XOR ^
#define false 0

#define ONE_CONTINUATION_BYTE 1
#define TWO_CONTINUATION_BYTE 2
#define THREE_CONTINUATION_BYTE 3

// 110xxxxx 10xxxxxx
// 1110xxxx 10xxxxxx 10xxxxxx
// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
// 0xxxxxxx (1 bytes)
int amount_of_utf_8(const unsigned char byte){
    if ((byte XOR MASK_HEADER_BYTE2) >> 5  == 0x00){
        return ONE_CONTINUATION_BYTE;
    } else if ((byte XOR MASK_HEADER_BYTE3) >> 4 == 0x00){
        return TWO_CONTINUATION_BYTE;
    }
    else if ((byte XOR MASK_HEADER_BYTE4) >> 3 == 0x00){
        return THREE_CONTINUATION_BYTE;
    }
    return 0;

}



int check_UTF_8 (const unsigned char byte, FILE* file){
    //Hvor mange bytes skal vi kigge fremad?
    int continuation_bytes_amount = amount_of_utf_8(byte);
    if (continuation_bytes_amount == 0){
        return false;
    }

    long pos_before_operation = ftell(file);
    unsigned char next_byte;
    for (int i = 0; i < continuation_bytes_amount; ++i)
    {
        // If we can't read the next byte or if the next byte is not a continuation
        // byte, then we have a malformed UTF-8 character
        unsigned char xor = (next_byte XOR MASK_CONTINUATION_BYTE);
        int is_not_continuation_byte = xor >> 6 != 0x00;
        // 10xxxxxx
        if (fread(&next_byte, sizeof(char), 1, file) == 0 || is_not_continuation_byte)
        {
            fseek(file, pos_before_operation, SEEK_SET);
            return 0;
        }
    }
    return 1;
}



int main(int argc, char* argv[]) {
    int retval = EXIT_SUCCESS;
    //No file given
    if (argc != 2){
        printf("Usage: file path");
        retval = EXIT_FAILURE;
        return retval;
    }

    // Not readable file
    if ((access(argv[1], R_OK)) == -1)
    {
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

    //

    //File type Empty
    unsigned char Char = fgetc(file);
    if (Char == EOF){
        type = EMPTY;
        //fclose(file) vi lukker den tilsidst
        //return retval vi lukker den tilsidst og return retval er kun for fejl.
    }

    //File type ASCII
    ungetc(Char, file); 

    while ((Char = fgetc(file)) != EOF){
        if (!check_ASCII(Char)) {
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
        if (!check_ASCII(Char) || !check_ISO_8859_1(Char)) {
            ungetc(Char, file); 
            type = DATA;
            break;
        }
        else{
            type = ISO8859;
        }
    }

    //File type UTF-8
    while ((Char = fgetc(file)) != EOF){
        if (!check_ASCII(Char) || !check_ISO_8859_1(Char) || !check_UTF_8(Char, file)) {
            ungetc(Char, file); 
            type = DATA;
            break;
        }
        else{
            type = UTF8;
        }
    }

    fclose(file);
    printf("%s: %s\n", argv[1], FILE_TYPE_STRINGS[type]);
    return retval;
}