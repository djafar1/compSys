#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#ifdef __APPLE__
#include "./endian.h"
#else
#include <endian.h>
#endif

#include "./networking.h"
#include "./sha256.h"

#define SALT_STORAGE "salt_storage.txt"

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
    char to_hash[strlen(password) + strlen(salt)];
    strcpy(to_hash, password);
    strcat(to_hash, salt);
    get_data_sha(to_hash, *hash, strlen(to_hash), SHA256_HASH_SIZE);
}

/*
* Easier to decode the response header
*/
void decoding_response_header(char* responseHeader, uint32_t* payloadLength, uint32_t* statusCode, uint32_t* blockNumber, 
uint32_t* blockCount, hashdata_t blockHash, hashdata_t totalHash)
{
    memcpy(payloadLength, responseHeader, 4);
    *payloadLength = ntohl(*payloadLength);
    memcpy(statusCode, responseHeader+4, 4);
    *statusCode = ntohl(*statusCode);
    memcpy(blockNumber, responseHeader+8, 4);
    *blockNumber = ntohl(*blockNumber);
    memcpy(blockCount, responseHeader+12, 4);
    *blockCount = ntohl(*blockCount);
    memcpy(blockHash, responseHeader + 16, sizeof(hashdata_t));
    memcpy(totalHash, responseHeader + 48, sizeof(hashdata_t));
}

/*
 * Making request_t struct to send to server. 
 */
Request_t get_request(char* username, char* password, char* salt, char* to_get){
    hashdata_t hash;
    get_signature(password, salt, &hash);
    Request_t request;
    strncpy(request.header.username, username, USERNAME_LEN);
    memcpy(request.header.salted_and_hashed, hash, SHA256_HASH_SIZE);
    request.header.length = htonl(strlen(to_get));
    strncpy(request.payload, to_get, PATH_LEN);
    return request;
}

/*
* Compare block hash from response header with payload hashed
*/
int compare_block_hash(hashdata_t serverhash, char *payload){
    hashdata_t payloadhashed;
    size_t payloadLength = strlen(payload);
    get_data_sha(payload, payloadhashed, payloadLength, SHA256_HASH_SIZE);
    return memcmp(serverhash, payloadhashed, SHA256_HASH_SIZE);
}
/*
* Compare total hash from response header with file hashed
*/
int compare_file_hash(hashdata_t totalHash, char* to_get){
    hashdata_t filehashed;
    get_file_sha(to_get, filehashed, SHA256_HASH_SIZE);
    return memcmp(totalHash, filehashed, SHA256_HASH_SIZE);
}

/*
 * Register a new user with a server by sending the username and signature to 
 * the server
 */
void register_user(char* username, char* password, char* salt)
{
    compsys_helper_state_t state;
    Request_t request; 

    //Using get_request to make request_t struct
    request = get_request(username, password, salt, "");

    // Open the network connection
    network_socket = compsys_helper_open_clientfd(server_ip, server_port);

    compsys_helper_readinitb(&state, network_socket);

    // Send the request to the server about registering the username and signature
    compsys_helper_writen(network_socket, &request, REQUEST_HEADER_LEN);

    //reading response from server
    //responseheader from server, with the size of responseheaderlen
    char responseHeader[RESPONSE_HEADER_LEN];

    // Read response from server, into the responseHeader, which would be the lenght of Response_header_len
    // So basically we only read the first 4+4+4+4+32+32, which is the responseheader.
    compsys_helper_readnb(&state, responseHeader, RESPONSE_HEADER_LEN);

    // We declare the variables which will store the 
    // different decoded information from the response header.
    uint32_t payloadLenght, statusCode, blockNumber, blockCount;
    hashdata_t blockHash, totalHash;

    // Decoding the response header using our function, and store it in the variables.
    decoding_response_header(responseHeader, &payloadLenght, &statusCode,
        &blockNumber, &blockCount, blockHash, totalHash);

    //Making a new buffer, of size payloadlenght
    char payload[payloadLenght+1];

    //Reading payload from the server using the lenght, into the buffer payload.
    compsys_helper_readnb(&state, payload, payloadLenght);

    //Setting last character to null byte
    payload[payloadLenght] = '\0';

    //Printing it the payload.
    printf("%s\n", payload);
    
    //Comparing hash from block with hashed payload
    if(compare_block_hash(blockHash, payload) != 0){
        printf("Hash from response header, do not match the hashed payload\n");
    }
    
    // Close the network connection
    close(network_socket);
}

/*
 * Get a file from the server by sending the username and signature, along with
 * a file path. Note that this function should be able to deal with both small 
 * and large files. 
 */
void get_file(char* username, char* password, char* salt, char* to_get)
{
    compsys_helper_state_t state;
    Request_t request;
    // Variable for checking whether the hash is identicial in each block.
    int identicalhash = 0;

    request = get_request(username, password, salt, to_get);
    // Open the network connection
    network_socket = compsys_helper_open_clientfd(server_ip, server_port);
    compsys_helper_readinitb(&state, network_socket);
    compsys_helper_writen(network_socket, &request, sizeof(Request_t));

    // Buffer for responseHeader
    char responseHeader[RESPONSE_HEADER_LEN];
    // Reading the responseHeader into the buffer
    compsys_helper_readnb(&state, responseHeader, RESPONSE_HEADER_LEN);

    // We declare the variables which will store the
    // different decoded information from the response header.
    uint32_t payloadLength, statusCode, blockNumber, blockCount;
    hashdata_t blockHash, totalHash;
    // Decoding the response header using our function, and store it in the variables.
    decoding_response_header(responseHeader, &payloadLength, &statusCode,
        &blockNumber, &blockCount, blockHash, totalHash);

    // If the statuscode from the responseheader is not okay, print payload and exit.
    if (statusCode != 1){
        char payload[payloadLength+1];
        compsys_helper_readnb(&state, payload, payloadLength);
        payload[payloadLength] = '\0';
        printf("Got unexpected status code: %d\n", statusCode);
        printf("%s\n", payload);

        // Close the network connection
        close(network_socket);
        return;
    }

    // Making a new file and opening it to write in binary.
    FILE* file = fopen(to_get, "wb");
    if (file == NULL) {
        fprintf(stderr, "Malloc failed for node\n");
        exit(EXIT_FAILURE);
    }

    // Make an array to store the blocks in, later used to write it.
    char** blocks = malloc(blockCount * sizeof(char*));
    if (blocks == NULL) {
        fprintf(stderr, "Malloc failed for node\n");
        exit(EXIT_FAILURE);
    }

    // Since we need to read the response header, to determine the amount of blocks
    // We need to manuel write in the first payload into the array of blocks
    char payload1[payloadLength + 1];
    compsys_helper_readnb(&state, payload1, payloadLength);
    blocks[blockNumber] = malloc(payloadLength + 1);
    // Copy the payload to the block
    payload1[payloadLength] = '\0';
    memcpy(blocks[blockNumber], payload1, payloadLength + 1);
    // Compare hash from blockhash from respose header, with payload hashed.
    if (compare_block_hash(blockHash, blocks[blockNumber]) != 0){
        identicalhash = 1;
    }
    // Read each subsequent block and store it in the array excluding first read blocknumber earlier. 
    // That is why we typed - 1.
    for (size_t i = 0; i < blockCount - 1; ++i) {
        compsys_helper_readnb(&state, responseHeader, RESPONSE_HEADER_LEN);
        decoding_response_header(responseHeader, &payloadLength, &statusCode,
            &blockNumber, &blockCount, blockHash, totalHash);
        char payload[payloadLength + 1];
        compsys_helper_readnb(&state, payload, payloadLength);

        // Allocate memory for the block/payload
        blocks[blockNumber] = malloc(payloadLength + 1);
        payload[payloadLength] = '\0';

        if (blocks[blockNumber] == NULL) {
            fprintf(stderr, "Malloc failed for node\n");
            exit(EXIT_FAILURE);
        }

        // Copy the payload to the block using the index of the blockNumber.
        memcpy(blocks[blockNumber], payload, payloadLength + 1);

        // Comparing each hash from the blockhash and payload
        if (compare_block_hash(blockHash, blocks[blockNumber]) != 0){
            identicalhash = 1;
        }

    }

    // Write blocks to the file in the correct order
    // Free allocated memory for blocks
    for (size_t i = 0; i < blockCount; ++i) {
        size_t blockLength = strlen(blocks[i]);
        fwrite(blocks[i], sizeof(char), blockLength, file);
        free(blocks[i]);
    }
    free(blocks);
    // Close the file
    fclose(file);
    printf("Retrieved data written to '%s'.\n", to_get);
    if(identicalhash != 0 || (compare_file_hash(totalHash, to_get)) != 0){
        printf("Hash from response header, do not match the hashed of either individual block or file \n");
    }
    // Close the network connection
    close(network_socket);
}

void generate_salt_and_save(const char* username, char* user_salt)
{
    //Randomly generate a salt for new users.
    srand(time(0));
    for (int i=0; i<SALT_LEN; i++)
    {  
        user_salt[i] = 'a' + (rand() % 26);
    }
    user_salt[SALT_LEN] = '\0';
    //Opening file with mode "a" to append at the end of file.
    FILE* file = fopen(SALT_STORAGE, "a");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }
    // Write string with username and user_salt to file/stream
    fprintf(file, "%s %s\n", username, user_salt);

    //Close file
    fclose(file);
}

void check_for_existing_salt(const char* username, char* user_salt)
{
    //Opening file 
    FILE* file = fopen(SALT_STORAGE, "r");
    if (file == NULL) {
        //If can't open try to create a new file
        file = fopen(SALT_STORAGE, "w");
        if (file == NULL) {
            fprintf(stderr, "Error creating file\n");
            exit(EXIT_FAILURE);    
        }       
        fclose(file);
        if (file == NULL){
            fprintf(stderr, "Error opening file\n");
            exit(EXIT_FAILURE);
        }
    }
    // Defining variables to store the username and salt read from file temporarily.
    char saved_username[USERNAME_LEN];
    char saved_salt[SALT_LEN + 1];

    // Using fscanf to read line by line in the file, 
    // Reading at most the lenght of username 16, and salt 64.
    while (fscanf(file, "%16s %64s", saved_username, saved_salt) == 2) {
        // Check if the username in file matches the username typed
        if (strcmp(username, saved_username) == 0) {
            // If it matches update value of user_salt with storaged salt.
            strcpy(user_salt, saved_salt);
            fclose(file);
            return;
        }
    }
    // If no match was found for the username in storage, then generate new salt for that username
    fclose(file);
    generate_salt_and_save(username, user_salt);
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

    // Checking for existing salt for the entered username if no
    // match then generate new random salt.
    check_for_existing_salt(username, user_salt);

    // Trying to register user, if already registered, will just proceed
    register_user(username, password, user_salt);

    char to_get[PATH_LEN];
    while (1){
        fprintf(stdout, "Type the name of a file to be retrieved, or 'quit' to quit:\n");
        scanf("%128s", to_get);
        while ((c = getchar()) != '\n' && c != EOF);
        if (strcmp(to_get, "quit") == 0){
            break;
        }
        //Get a file from server by sending the username, signature and file path, that
        //user typed in
        get_file(username, password, user_salt, to_get);
    }
    exit(EXIT_SUCCESS);
}