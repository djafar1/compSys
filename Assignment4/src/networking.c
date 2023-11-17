#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"

char server_ip[IP_LEN];
char server_port[PORT_LEN];
char my_ip[IP_LEN];
char my_port[PORT_LEN];

int c;
int network_socket;
/*
 * Gets a sha256 hash of specified data, sourcedata. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_data_sha(const char* sourcedata, hashdata_t hash, uint32_t data_size, 
    int hash_size)
{
  SHA256_CTX shactx;
  unsigned char shabuffer[hash_size];
  sha256_init(&shactx);
  sha256_update(&shactx, sourcedata, data_size);
  sha256_final(&shactx, shabuffer);

  for (int i=0; i<hash_size; i++)
  {
    hash[i] = shabuffer[i];
  }
}

/*
 * Gets a sha256 hash of specified data file, sourcefile. The hash itself is
 * placed into the given variable 'hash'. Any size can be created, but a
 * a normal size for the hash would be given by the global variable
 * 'SHA256_HASH_SIZE', that has been defined in sha256.h
 */
void get_file_sha(const char* sourcefile, hashdata_t hash, int size)
{
    int casc_file_size;

    FILE* fp = fopen(sourcefile, "rb");
    if (fp == 0)
    {
        printf("Failed to open source: %s\n", sourcefile);
        return;
    }

    fseek(fp, 0L, SEEK_END);
    casc_file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char buffer[casc_file_size];
    fread(buffer, casc_file_size, 1, fp);
    fclose(fp);

    get_data_sha(buffer, hash, casc_file_size, size);
}
/*
 * Combine a password and salt together and hash the result to form the 
 * 'signature'. The result should be written to the 'hash' variable. Note that 
 * as handed out, this function is never called. You will need to decide where 
 * it is sensible to do so.
 */
void get_signature(char* password, char* salt, hashdata_t* hash)
{
    // Your code here. This function has been added as a guide, but feel free 
    // to add more, or work in other parts of the code
    char to_hash[strlen(password) + strlen(salt)];
    // TODO Put some code in here so that to_hash contains the password and 
    // salt and is then hashed
    strcpy(to_hash, password);
    strcat(to_hash, salt);
    get_data_sha(to_hash, hash, strlen(to_hash), SHA256_HASH_SIZE);
    // You can use this to confirm that you are hashing what you think you are
    // hashing
    /*
    for (uint8_t i=0; i<strlen(to_hash); i++)
    {
        printf("[%c]", to_hash[i]);
    }
    printf("\n");*/
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* salt)
{
    compsys_helper_state_t state;
    hashdata_t hash;
    get_signature(password, salt, hash);
    RequestHeader_t header;

    memcpy(header.username, username, USERNAME_LEN);
    // Alternative: strncpy(request.username, username, USERNAME_LEN);
    //request.username[USERNAME_LEN] = '\0';

    memcpy(header.salted_and_hashed, hash, SHA256_HASH_SIZE);
    //Alternative: strncpy(request.salted_and_hashed, hash, SHA256_HASH_SIZE);
    //request.salted_and_hashed[SHA256_HASH_SIZE] = '\0';

    // the lenght of the payload is 0, since we are not sending anything.
    header.length = 0;

    compsys_helper_readinitb(&state, network_socket);
    // Send the request to the server about registering the username and signature
    compsys_helper_writen(network_socket, &header, REQUEST_HEADER_LEN);

    //reading response from server

    //responseheader from server, with the size of responseheaderlen

    char responseHeader[RESPONSE_HEADER_LEN];
    // Read response from server, into the responseHeader, which would be the lenght of Response_header_len
    // So basically we only read the first 4+4+4+4+32+32, which is the responseheader.
    compsys_helper_readnb(&state, responseHeader, RESPONSE_HEADER_LEN);

    //Creating a uint32_t which holds the lenght of the payload
    uint32_t payloadLenght;
    //Putting the first four bytes into the payloadLenght, since we know that the first four bytes tells us the lenght of the payload.
    memcpy(&payloadLenght, responseHeader, 4);
    //Translates an unsigned long integer(network byte order) into host byte order.
    payloadLenght = ntohl(payloadLenght);

    //Making a new buffer, of size payloadlenght
    char payload[payloadLenght+1];

    //Reading payload from the server using the lenght, into the buffer payload.
    compsys_helper_readnb(&state, payload, payloadLenght);

    //Setting last character to null byte
    payload[payloadLenght] = '\0';

    //Printing it the payload.
    printf("%s\n", payload);

    //Tryna match the two hashes.
    // VIRKER IKKE PRØVER AT TAGE HASH FRA RESPONSE HEADER OG LAVE HASH AF PAYLOAD OG SAMMENLIGNE
    // MEN DE ER VIDT FORSKELLIGE :(, DET SKAL DU FIKSE FELICIA
    /*
    hashdata_t totalHash;
    memcpy(&totalHash, responseHeader + 58, sizeof(hashdata_t));
    hashdata_t hashofpayload;
    get_data_sha(payload-1, hashofpayload, payloadLenght, SHA256_HASH_SIZE);
    hashofpayload[SHA256_HASH_SIZE] = '\0';
    printf("Total Hash (UTF-8): ");
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        printf("%02x", totalHash[i]);
    }
    printf("\n");

    printf("Hash of Payload (UTF-8): ");
    for (int i = 0; i < SHA256_HASH_SIZE; i++) {
        printf("%02x", hashofpayload[i]);
    }
    printf("\n");
    printf("Length of Total Hash: %lu\n", sizeof(totalHash));
    printf("Length of Hash of Payload: %lu\n", strlen(hashofpayload));
    */
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get)
{
    compsys_helper_state_t state;
    hashdata_t hash;
    get_signature(password, salt, hash);
    Request_t request;
    strncpy(request.header.username, username, USERNAME_LEN);
    memcpy(request.header.salted_and_hashed, hash, SHA256_HASH_SIZE);
    request.header.length = 2147483648;
    // hostbyte order til netwrok byte order htonl, ntohl
    strncpy(request.payload, to_get, PATH_LEN);
    compsys_helper_writen(network_socket, &request, sizeof(Request_t));

}

int main(int argc, char **argv)
{
    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    // Read in configuration options. Should include a client_directory, 
    // client_ip, client_port, server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, CLIENT_IP)) {
            memcpy(my_ip, &buffer[strlen(CLIENT_IP)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_IP));
            if (!is_valid_ip(my_ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, CLIENT_PORT)) {
            memcpy(my_port, &buffer[strlen(CLIENT_PORT)], 
                strcspn(buffer, "\r\n")-strlen(CLIENT_PORT));
            if (!is_valid_port(my_port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", my_port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_IP)) {
            memcpy(server_ip, &buffer[strlen(SERVER_IP)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_IP));
            if (!is_valid_ip(server_ip)) {
                fprintf(stderr, ">> Invalid server IP: %s\n", server_ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, SERVER_PORT)) {
            memcpy(server_port, &buffer[strlen(SERVER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(SERVER_PORT));
            if (!is_valid_port(server_port)) {
                fprintf(stderr, ">> Invalid server port: %s\n", server_port);
                exit(EXIT_FAILURE);
            }
        }        
    }
    fclose(fp);

    fprintf(stdout, "Client at: %s:%s\n", my_ip, my_port);
    fprintf(stdout, "Server at: %s:%s\n", server_ip, server_port);

    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
    char user_salt[SALT_LEN+1];
    
    fprintf(stdout, "Enter a username to proceed: ");
    scanf("%16s", username);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up username string as otherwise some extra chars can sneak in.
    for (int i=strlen(username); i<USERNAME_LEN; i++)
    {
        username[i] = '\0';
    }
 
    fprintf(stdout, "Enter your password to proceed: ");
    scanf("%16s", password);
    while ((c = getchar()) != '\n' && c != EOF);
    // Clean up password string as otherwise some extra chars can sneak in.
    for (int i=strlen(password); i<PASSWORD_LEN; i++)
    {
        password[i] = '\0';
    }

    // Note that a random salt should be used, but you may find it easier to
    // repeatedly test the same user credentials by using the hard coded value
    // below instead, and commenting out this randomly generating section.
    /*
    for (int i=0; i<SALT_LEN; i++)
    {
        user_salt[i] = 'a' + (random() % 26);
    }
    user_salt[SALT_LEN] = '\0';*/
    strncpy(user_salt, 
        "0123456789012345678901234567890123456789012345678901234567890123\0", 
        SALT_LEN+1);

    fprintf(stdout, "Using salt: %s\n", user_salt);

    network_socket = compsys_helper_open_clientfd(server_ip, server_port);
    // The following function calls have been added as a structure to a 
    // potential solution demonstrating the core functionality. Feel free to 
    // add, remove or otherwise edit. Note that if you are creating a system 
    // for user-interaction the following lines will almost certainly need to 
    // be removed/altered.

    // Register the given user. As handed out, this line will run every time 
    // this client starts, and so should be removed if user interaction is 
    // added
    register_user(username, password, user_salt);

    // Retrieve the smaller file, that doesn't not require support for blocks. 
    // As handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added
    //get_file(username, password, user_salt, "tiny.txt");

    // Retrieve the larger file, that requires support for blocked messages. As
    // handed out, this line will run every time this client starts, and so 
    // should be removed if user interaction is added
    //get_file(username, password, user_salt, "hamlet.txt");
    close(network_socket);
    exit(EXIT_SUCCESS);
}