#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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

#include "./peer.h"
#include "./sha256.h"


// Global variables to be used by both the server and client side of the peer.
// Some of these are not currently used but should be considered STRONG hints
PeerAddress_t *my_address;

pthread_mutex_t network_mutex = PTHREAD_MUTEX_INITIALIZER;
PeerAddress_t** network = NULL;
uint32_t peer_count = 0;

pthread_mutex_t retrieving_mutex = PTHREAD_MUTEX_INITIALIZER;
FilePath_t** retrieving_files = NULL;
uint32_t file_count = 0;


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
 * A simple min function, which apparently C doesn't have as standard
 */
uint32_t min(int a, int b)
{
    if (a < b) 
    {
        return a;
    }
    return b;
}

/*
 * Select a peer from the network at random, without picking the peer defined
 * in my_address
 */
void get_random_peer(PeerAddress_t* peer_address)
{ 
    PeerAddress_t** potential_peers = malloc(sizeof(PeerAddress_t*));
    uint32_t potential_count = 0; 
    for (uint32_t i=0; i<peer_count; i++)
    {
        if (strcmp(network[i]->ip, my_address->ip) != 0 
                || strcmp(network[i]->port, my_address->port) != 0 )
        {
            potential_peers = realloc(potential_peers, 
                (potential_count+1) * sizeof(PeerAddress_t*));
            potential_peers[potential_count] = network[i];
            potential_count++;
        }
    }

    if (potential_count == 0)
    {
        printf("No peers to connect to. You probably have not implemented "
            "registering with the network yet.\n");
    }

    uint32_t random_peer_index = rand() % potential_count;

    memcpy(peer_address->ip, potential_peers[random_peer_index]->ip, IP_LEN);
    memcpy(peer_address->port, potential_peers[random_peer_index]->port, 
        PORT_LEN);

    free(potential_peers);

    printf("Selected random peer: %s:%s\n", 
        peer_address->ip, peer_address->port);
}

void handle_reply_fromserver(char* reply_body, uint32_t reply_lenght){
    //The reply_body/payload from the server includes our address
    //So when we go into the for loop we add it back in the order the we receive it from the server peer
    //The lenght of the payload divided by 20 is equal to the amount of peers in the network
    peer_count = reply_lenght/20;
    //Realloc more space for the network
    assert(pthread_mutex_lock(&network_mutex) == 0);
    network = realloc(network, peer_count * sizeof(PeerAddress_t*));
    if (network == NULL) {
        fprintf(stderr, "Realloc failed for network\n");
        assert(pthread_mutex_unlock(&network_mutex) == 0);
        exit(EXIT_FAILURE);
    }

    for (uint32_t i=0; i<peer_count; i++){
        PeerAddress_t* NewAdress = malloc(sizeof(PeerAddress_t));
        char ip[IP_LEN];
        memcpy(ip, reply_body+(i*20), IP_LEN);
        char portstr[PORT_LEN];
        uint32_t port;
        memcpy(&port, reply_body+(i*20+16), 4);
        port = ntohl(port);
        sprintf(portstr, "%d", port);
        memcpy(NewAdress->ip, ip, IP_LEN);
        memcpy(NewAdress->port, portstr, PORT_LEN);
        network[i] = NewAdress;
    }
    assert(pthread_mutex_unlock(&network_mutex) == 0);

    assert(pthread_mutex_lock(&network_mutex) == 0);
    printf("Got network:");
    for (uint32_t i = 0; i < peer_count; i++){
        printf(" %s:%s,", network[i]->ip, network[i]->port);
    }
    printf("\n");
    assert(pthread_mutex_unlock(&network_mutex) == 0);
}

/*
 * Send a request message to another peer on the network. Unless this is 
 * specifically an 'inform' message as described in the assignment handout, a 
 * reply will always be expected.
 */
void send_message(PeerAddress_t peer_address, int command, char* request_body)
{
    fprintf(stdout, "Connecting to server at %s:%s to run command %d (%s)\n", 
        peer_address.ip, peer_address.port, command, request_body);

    compsys_helper_state_t state;
    char msg_buf[MAX_MSG_LEN];
    FILE* fp;

    // Setup the eventual output file path. This is being done early so if 
    // something does go wrong at this stage we can avoid all that pesky 
    // networking
    char output_file_path[strlen(request_body)+1];
    if (command == COMMAND_RETREIVE)
    {     
        strcpy(output_file_path, request_body);

        if (access(output_file_path, F_OK ) != 0 ) 
        {
            fp = fopen(output_file_path, "a");
            fclose(fp);
        }
    }

    // Setup connection
    int peer_socket = compsys_helper_open_clientfd(peer_address.ip, peer_address.port);
    compsys_helper_readinitb(&state, peer_socket);

    // Construct a request message and send it to the peer
    struct RequestHeader request_header;
    strncpy(request_header.ip, my_address->ip, IP_LEN);
    request_header.port = htonl(atoi(my_address->port));
    request_header.command = htonl(command);
    // Distinquish between inform or request and register.
    if (command == COMMAND_INFORM){
        request_header.length = htonl(20);
    }
    else {
        request_header.length = htonl(strlen(request_body));
    }  
    memcpy(msg_buf, &request_header, REQUEST_HEADER_LEN);
    if (command == COMMAND_INFORM){
        memcpy(msg_buf+REQUEST_HEADER_LEN, request_body, 20);

    }
    else {
        memcpy(msg_buf+REQUEST_HEADER_LEN, request_body, strlen(request_body));
    }
    if (command == COMMAND_INFORM){
        compsys_helper_writen(peer_socket, msg_buf, REQUEST_HEADER_LEN+20);
    }
    else {
        compsys_helper_writen(peer_socket, msg_buf, REQUEST_HEADER_LEN+strlen(request_body));
    }

    // We don't expect replies to inform messages so we're done here
    if (command == COMMAND_INFORM)
    {
        close(peer_socket);
        return;
    }

    // Read a reply
    compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

    // Extract the reply header 
    char reply_header[REPLY_HEADER_LEN];
    memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

    uint32_t reply_length = ntohl(*(uint32_t*)&reply_header[0]);
    uint32_t reply_status = ntohl(*(uint32_t*)&reply_header[4]);
    uint32_t this_block = ntohl(*(uint32_t*)&reply_header[8]);
    uint32_t block_count = ntohl(*(uint32_t*)&reply_header[12]);
    hashdata_t block_hash;
    memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
    hashdata_t total_hash;
    memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

    // Determine how many blocks we are about to recieve
    hashdata_t ref_hash;
    memcpy(ref_hash, &total_hash, SHA256_HASH_SIZE);
    uint32_t ref_count = block_count;

    // Loop until all blocks have been recieved
    for (uint32_t b=0; b<ref_count; b++)
    {
        // Don't need to re-read the first block
        if (b > 0)
        {
            // Read the response
            compsys_helper_readnb(&state, msg_buf, REPLY_HEADER_LEN);

            // Read header
            memcpy(reply_header, msg_buf, REPLY_HEADER_LEN);

            // Parse the attributes
            reply_length = ntohl(*(uint32_t*)&reply_header[0]);
            reply_status = ntohl(*(uint32_t*)&reply_header[4]);
            this_block = ntohl(*(uint32_t*)&reply_header[8]);
            block_count = ntohl(*(uint32_t*)&reply_header[12]);

            memcpy(block_hash, &reply_header[16], SHA256_HASH_SIZE);
            memcpy(total_hash, &reply_header[48], SHA256_HASH_SIZE);

            // Check we're getting consistent results
            if (ref_count != block_count)
            {
                fprintf(stdout, 
                    "Got inconsistent block counts between blocks\n");
                close(peer_socket);
                return;
            }

            for (int i=0; i<SHA256_HASH_SIZE; i++)
            {
                if (ref_hash[i] != total_hash[i])
                {
                    fprintf(stdout, 
                        "Got inconsistent total hashes between blocks\n");
                    close(peer_socket);
                    return;
                }
            }
        }

        // Check response status
        if (reply_status != STATUS_OK)
        {
            if (command == COMMAND_REGISTER && reply_status == STATUS_PEER_EXISTS)
            {
                printf("Peer already exists\n");
            }
            else
            {
                printf("Got unexpected status %d\n", reply_status);
                close(peer_socket);
                return;
            }
        }

        // Read the payload
        char payload[reply_length+1];
        compsys_helper_readnb(&state, msg_buf, reply_length);
        memcpy(payload, msg_buf, reply_length);
        payload[reply_length] = '\0';
        
        // Check the hash of the data is as expected
        hashdata_t payload_hash;
        get_data_sha(payload, payload_hash, reply_length, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (payload_hash[i] != block_hash[i])
            {
                fprintf(stdout, "Payload hash does not match specified\n");
                close(peer_socket);
                return;
            }
        }

        // If we're trying to get a file, actually write that file
        if (command == COMMAND_RETREIVE)
        {
            // Check we can access the output file
            fp = fopen(output_file_path, "r+b");
            if (fp == 0)
            {
                printf("Failed to open destination: %s\n", output_file_path);
                close(peer_socket);
            }

            uint32_t offset = this_block * (MAX_MSG_LEN-REPLY_HEADER_LEN);
            fprintf(stdout, "Block num: %d/%d (offset: %d)\n", this_block+1, 
                block_count, offset);
            fprintf(stdout, "Writing from %d to %d\n", offset, 
                offset+reply_length);

            // Write data to the output file, at the appropriate place
            fseek(fp, offset, SEEK_SET);
            fputs(payload, fp);
            fclose(fp);
        }
    }

    // Confirm that our file is indeed correct
    if (command == COMMAND_RETREIVE)
    {
        fprintf(stdout, "Got data and wrote to %s\n", output_file_path);

        // Finally, check that the hash of all the data is as expected
        hashdata_t file_hash;
        get_file_sha(output_file_path, file_hash, SHA256_HASH_SIZE);

        for (int i=0; i<SHA256_HASH_SIZE; i++)
        {
            if (file_hash[i] != total_hash[i])
            {
                fprintf(stdout, "File hash does not match specified for %s\n", 
                    output_file_path);
                close(peer_socket);
                return;
            }
        }
    }

    // If we are registering with the network we should note the complete 
    // network reply
    char* reply_body = malloc(reply_length + 1);
    memset(reply_body, 0, reply_length + 1);
    memcpy(reply_body, msg_buf, reply_length);

    if (reply_status == STATUS_OK)
    {
        if (command == COMMAND_REGISTER)
        {
            // Calls our helper function to decode and update global variables
            handle_reply_fromserver(reply_body, reply_length);
        }
    } 
    else
    {
        printf("Got response code: %d, %s\n", reply_status, reply_body);

    }
    free(reply_body);
    close(peer_socket);
}
/*
 * Function to act as thread for all required client interactions. This thread 
 * will be run concurrently with the server_thread but is finite in nature.
 * 
 * This is just to register with a network, then download two files from a 
 * random peer on that network. As in A3, you are allowed to use a more 
 * user-friendly setup with user interaction for what files to retrieve if 
 * preferred, this is merely presented as a convienient setup for meeting the 
 * assignment tasks
 */ 
void* client_thread(void* thread_args)
{
    struct PeerAddress *peer_address = thread_args;

    // Register the given user
    //send_message(*peer_address, COMMAND_REGISTER, "\0");

    // Update peer_address with random peer from network
    //get_random_peer(peer_address);

    // Retrieve the smaller file, that doesn't not require support for blocks
    send_message(*peer_address, COMMAND_RETREIVE, "tiny.txt");

    // Update peer_address with random peer from network
    //get_random_peer(peer_address);

    // Retrieve the larger file, that requires support for blocked messages
    send_message(*peer_address, COMMAND_RETREIVE, "hamlet.txt");

    //Variable for path and for reading the input and making user interaction
    int c;
    char to_get[PATH_LEN];
 
    //The while loop which allows for user interaction.
    while (1){
        printf("Type the name of a file to be retrieved, or 'quit' to quit:\n");
        scanf("%128s", to_get);
        while ((c = getchar()) != '\n' && c != EOF);
        if (strcmp(to_get, "quit") == 0){
            break;
        }
        // Update peer_address with random peer from network        
        //get_random_peer(peer_address);

        //Retrieve the larger file, that the client/peer request.
        send_message(*peer_address, COMMAND_RETREIVE, to_get);
    }
    printf("Shutting down client thread.\n");

    return NULL;
}

/*
* Handle the inform of the new peer using send message and the whole network
*/
void inform_peers(char* client_ip, int client_port_int){
    char request_body[IP_LEN + sizeof(uint32_t) + 1];

    // Client port in network byte order
    uint32_t client_port = htonl(client_port_int);
    
    memcpy(request_body, client_ip, IP_LEN);
    memcpy(request_body + IP_LEN, &client_port, sizeof(uint32_t));

    request_body[sizeof(request_body) - 1] = '\0';
    for (uint32_t i=0; i<peer_count - 1; i++)
    {   
        assert(pthread_mutex_lock(&network_mutex) == 0);
        if (strcmp(network[i]->ip, my_address->ip) != 0 || strcmp(network[i]->port, my_address->port) != 0 ){
            send_message(*network[i], COMMAND_INFORM, request_body);
        }
        assert(pthread_mutex_unlock(&network_mutex) == 0);
    }
}

/*
 * Handle any 'register' type requests, as defined in the asignment text. This
 * should always generate a response.
 */
void handle_register(int connfd, char* client_ip, int client_port_int)
{
    //Basically when a another peer / client writes to us
    //we act as a server, so it wants to register with us
    //then we ofc register the 
    PeerAddress_t* new_adress = malloc(sizeof(PeerAddress_t));
    char portstr[PORT_LEN];
    sprintf(portstr, "%d", client_port_int);
    memcpy(new_adress->ip, client_ip, IP_LEN);
    memcpy(new_adress->port, portstr, PORT_LEN);
    int exist = 0;
    uint32_t status;

    printf("Got registration message from %s:%s\n", new_adress->ip, new_adress->port);

    for (uint32_t i = 0; i < peer_count; i++)
    {   
        assert(pthread_mutex_lock(&network_mutex) == 0);
        if (strcmp(network[i]->ip, new_adress->ip) == 0 && strcmp(network[i]->port, new_adress->port) == 0) {
            exist = 1;
            status = STATUS_PEER_EXISTS;
        }
        assert(pthread_mutex_unlock(&network_mutex) == 0);
    }
    //If the case that the peer already exists
    if (exist == 1){
        // The struct for reply header
        ReplyHeader_t reply_header; 
        reply_header.status = htonl(status); // The status 
        reply_header.block_count = htonl(1);
        reply_header.this_block = htonl(0);   
        reply_header.length = htonl(0);
        hashdata_t hash;
        get_data_sha("", hash, 0, SHA256_HASH_SIZE);
        memcpy(reply_header.block_hash, hash, SHA256_HASH_SIZE);
        memcpy(reply_header.total_hash, hash, SHA256_HASH_SIZE);

        //Send reply header back to server
        compsys_helper_writen(connfd, &reply_header, REPLY_HEADER_LEN);

        printf("Cannot register peer %s:%s, already exists\n", new_adress->ip, new_adress->port);

        //Free the allocated memory for the new_adress
        free(new_adress);
    }
    //If the case that the peer doesn't already exists
    if (exist == 0){
        status = STATUS_OK;
        char msg_buf[MAX_MSG_LEN];

        //Adding the new to peer the network
        peer_count++;
        assert(pthread_mutex_lock(&network_mutex) == 0);
        network = realloc(network, peer_count * sizeof(PeerAddress_t*));
        if (network == NULL) {
            fprintf(stderr, "Realloc failed for network\n");
            exit(EXIT_FAILURE);
        }
        network[peer_count-1] = new_adress;
        assert(pthread_mutex_unlock(&network_mutex) == 0);

        //Making a struct to hold the reply to the peer that registered with us
        assert(pthread_mutex_lock(&network_mutex) == 0);
        NetworkAddress_t payload[peer_count];
        for (uint32_t i = 0; i < peer_count; i++) {
            strncpy(payload[i].ip, network[i]->ip, IP_LEN);
            payload[i].port = htonl(atoi(network[i]->port));
        }
        assert(pthread_mutex_unlock(&network_mutex) == 0);


        ReplyHeader_t reply_header; // The struct for reply header
        reply_header.status = htonl(status); // The status 
        reply_header.block_count = htonl(1);
        reply_header.this_block = htonl(0);
        reply_header.length = htonl(sizeof(NetworkAddress_t) * peer_count);
        hashdata_t hash;
        get_data_sha((char*)payload, hash, sizeof(NetworkAddress_t) * peer_count, SHA256_HASH_SIZE);

        memcpy(reply_header.block_hash, hash, SHA256_HASH_SIZE);
        memcpy(reply_header.total_hash, hash, SHA256_HASH_SIZE);

        memcpy(msg_buf, &reply_header, REPLY_HEADER_LEN);
        memcpy(msg_buf+REPLY_HEADER_LEN, &payload, sizeof(payload));
        
        compsys_helper_writen(connfd, msg_buf, REPLY_HEADER_LEN+(sizeof(payload)));

        // Using helper function to inform other peers about the newly added peer.
        inform_peers(client_ip, client_port_int);

        printf("Registered new peer %s:%s\n", new_adress->ip, new_adress->port);

    }
    //Just to print the complete network.
    assert(pthread_mutex_lock(&network_mutex) == 0);
    printf("Network is:");
    for (uint32_t i = 0; i < peer_count; i++){
        printf(" %s:%s,", network[i]->ip, network[i]->port);
    }
    printf("\n");
    assert(pthread_mutex_unlock(&network_mutex) == 0);
}

/*
 * Handle 'inform' type message as defined by the assignment text. These will 
 * never generate a response, even in the case of errors.
 */
void handle_inform(char* request)
{
    // In our code we assume that the first 16 bytes are for the IP
    // and the last 4 is for the port, so are allowed to hardcode it.
    char ip_address[IP_LEN];
    memcpy(ip_address, &request[0], IP_LEN);
    uint32_t port_int = ntohl(*(uint32_t*)&request[IP_LEN]); 
    PeerAddress_t* new_adress = malloc(sizeof(PeerAddress_t));
    char portstr[PORT_LEN];
    sprintf(portstr, "%d", port_int);
    memcpy(new_adress->ip, ip_address, IP_LEN);
    memcpy(new_adress->port, portstr, PORT_LEN);

    int exist = 0;
    assert(pthread_mutex_lock(&network_mutex) == 0);
    for (uint32_t i = 0; i < peer_count; i++)
    {
        if (strcmp(network[i]->ip, new_adress->ip) == 0 && strcmp(network[i]->port, new_adress->port) == 0) {
            exist = 1;
        }
    }
    if (exist == 0){
        printf("Informed of new peer %s:%s\n", new_adress->ip,new_adress->port);
        //Adding the new to peer the network
        peer_count++;
        network = realloc(network, peer_count * sizeof(PeerAddress_t*));
        if (network == NULL) {
            fprintf(stderr, "Realloc failed for network\n");
            exit(EXIT_FAILURE);
        }
        network[peer_count-1] = new_adress;
        assert(pthread_mutex_unlock(&network_mutex) == 0);
    }
    // If it already is somehow already registered in our network
    else {
        printf("The peer: %s:%s is already registered in our network \n", new_adress->ip, new_adress->port);
        free(new_adress);
        assert(pthread_mutex_unlock(&network_mutex) == 0);
    }
}

/*
 * Handle 'retrieve' type messages as defined by the assignment text. This will
 * always generate a response
 */
void handle_retreive(int connfd, char* request)
{

    // Check if the requested file exists
    FILE* file = fopen(request, "r");
    if (file == NULL) {
        ReplyHeader_t reply_header; 
        reply_header.status = htonl(STATUS_BAD_REQUEST); // The status 
        reply_header.block_count = htonl(1);
        reply_header.this_block = htonl(0);   
        reply_header.length = htonl(0);
        hashdata_t hash;
        get_data_sha("", hash, 0, SHA256_HASH_SIZE);
        memcpy(reply_header.block_hash, hash, SHA256_HASH_SIZE);
        memcpy(reply_header.total_hash, hash, SHA256_HASH_SIZE);

        //Send reply header back to server
        compsys_helper_writen(connfd, &reply_header, REPLY_HEADER_LEN);
        fprintf(stderr, "Bad Request (i.e. the request is coherent but cannot be servered as the file doesn't exist or is busy)\n");
        return;
    } 
    // Number of bytes in the file
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    //Dividing the (file_size+MAX_Payload) with Max_Payload, ensures that it rounds up to the nearest integer
    //Also in case that it is a small file it will ensure that atleast one block is made for it.
    uint32_t max_payload = MAX_MSG_LEN-REPLY_HEADER_LEN;
    uint32_t num_blocks = (file_size + max_payload) / (max_payload); 
    hashdata_t totalhash;
    get_file_sha(request, totalhash, SHA256_HASH_SIZE);

    printf("Sending request data from %s\n", request);
    for (uint32_t i = 0; i < num_blocks; i++)
    {
        char msg_buf[MAX_MSG_LEN];
        char payload[max_payload];
        //Reads max_payload into our buffer and also saves the number of bytes read.
        uint32_t payload_length = fread(payload, sizeof(char), max_payload, file);
        hashdata_t blockhash;
        get_data_sha(payload, blockhash, payload_length, SHA256_HASH_SIZE);
        ReplyHeader_t reply_header; // The struct for reply header

        reply_header.this_block = htonl(i);
        reply_header.block_count = htonl(num_blocks);
        reply_header.length = htonl(payload_length);
        reply_header.status = htonl(1);

        memcpy(reply_header.block_hash, blockhash, SHA256_HASH_SIZE);
        memcpy(reply_header.total_hash, totalhash, SHA256_HASH_SIZE);

        memcpy(msg_buf, &reply_header, REPLY_HEADER_LEN);
        memcpy(msg_buf+REPLY_HEADER_LEN, &payload, payload_length);

        compsys_helper_writen(connfd, msg_buf, REPLY_HEADER_LEN+payload_length);
        printf("Sending reply %d/%d with payload lenghth of %d\n", i+1,num_blocks, payload_length);
    }
    fclose(file);
}

/*
 * Handler for all server requests. This will call the relevent function based 
 * on the parsed command code
 */
void handle_server_request(int connfd)
{
    char msg_buf[MAX_MSG_LEN];
    compsys_helper_state_t state;
    compsys_helper_readinitb(&state, connfd);
    compsys_helper_readnb(&state, msg_buf, REQUEST_HEADER_LEN);
    char reply_header[REQUEST_HEADER_LEN];
    memcpy(reply_header, msg_buf, REQUEST_HEADER_LEN);
    char ip[IP_LEN];
    memcpy(ip, &reply_header[0], IP_LEN);
    uint32_t port = ntohl(*(uint32_t*)&reply_header[16]);
    uint32_t command = ntohl(*(uint32_t*)&reply_header[20]);
    uint32_t length = ntohl(*(uint32_t*)&reply_header[24]);
    char request[length+1];
    compsys_helper_readnb(&state, msg_buf, length);
    memcpy(request, msg_buf, length);
    request[length] = '\0';
    if (command == COMMAND_INFORM){
        handle_inform(request);
    }
    else if(command == COMMAND_REGISTER){
        handle_register(connfd, ip, port);
    }
    else if(command == COMMAND_RETREIVE){
        printf("Got request message from %s:%d\n", ip, port);
        handle_retreive(connfd, request);
    }
}

/*
 * Function to act as basis for running the server thread. This thread will be
 * run concurrently with the client thread, but is infinite in nature.
 */
void *server_thread()
{
    int listenfd;
    int connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    // Open listening socket
    listenfd = compsys_helper_open_listenfd(my_address->port);
    if (listenfd < 0) {
        fprintf(stderr, "Failed to open listening socket \n");
        exit(EXIT_FAILURE);
    }

    printf("Starting server at %s:%s\n", my_address->ip, my_address->port);
    
    while (1) {
        // Any incoming calls are handled in a new server thread
        //assert(pthread_mutex_lock(&network_mutex) == 0);
        clientlen = sizeof(struct sockaddr_storage);
        connfd = accept(listenfd, (struct sockaddr*)&clientaddr, &clientlen);
        if (connfd < 0) {
            fprintf(stderr, "Error when accepting connection\n");
            close(connfd);
            continue; 
        }
        printf("Accepted connection\n");

        // Handle the server request
        handle_server_request(connfd);

        // Close the connection
        close(connfd);
        //assert(pthread_mutex_unlock(&network_mutex) == 0); 
    }
    //Close the server
    close(listenfd);

    // This line will never be reached
    printf("Server thread done\n");
    return NULL;
}

int main(int argc, char **argv)
{
    // Initialise with known junk values, so we can test if these were actually
    // present in the config or not
    struct PeerAddress peer_address;
    memset(peer_address.ip, '\0', IP_LEN);
    memset(peer_address.port, '\0', PORT_LEN);
    memcpy(peer_address.ip, "x", 1);
    memcpy(peer_address.port, "x", 1);

    // Users should call this script with a single argument describing what 
    // config to use
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <config file>\n", argv[0]);
        exit(EXIT_FAILURE);
    } 

    my_address = (PeerAddress_t*)malloc(sizeof(PeerAddress_t));
    memset(my_address->ip, '\0', IP_LEN);
    memset(my_address->port, '\0', PORT_LEN);

    // Read in configuration options. Should include a client_ip, client_port, 
    // server_ip, and server_port
    char buffer[128];
    fprintf(stderr, "Got config path at: %s\n", argv[1]);
    FILE* fp = fopen(argv[1], "r");
    while (fgets(buffer, 128, fp)) {
        if (starts_with(buffer, MY_IP)) {
            memcpy(&my_address->ip, &buffer[strlen(MY_IP)], 
                strcspn(buffer, "\r\n")-strlen(MY_IP));
            if (!is_valid_ip(my_address->ip)) {
                fprintf(stderr, ">> Invalid client IP: %s\n", my_address->ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, MY_PORT)) {
            memcpy(&my_address->port, &buffer[strlen(MY_PORT)], 
                strcspn(buffer, "\r\n")-strlen(MY_PORT));
            if (!is_valid_port(my_address->port)) {
                fprintf(stderr, ">> Invalid client port: %s\n", 
                    my_address->port);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_IP)) {
            memcpy(peer_address.ip, &buffer[strlen(PEER_IP)], 
                strcspn(buffer, "\r\n")-strlen(PEER_IP));
            if (!is_valid_ip(peer_address.ip)) {
                fprintf(stderr, ">> Invalid peer IP: %s\n", peer_address.ip);
                exit(EXIT_FAILURE);
            }
        }else if (starts_with(buffer, PEER_PORT)) {
            memcpy(peer_address.port, &buffer[strlen(PEER_PORT)], 
                strcspn(buffer, "\r\n")-strlen(PEER_PORT));
            if (!is_valid_port(peer_address.port)) {
                fprintf(stderr, ">> Invalid peer port: %s\n", 
                    peer_address.port);
                exit(EXIT_FAILURE);
            }
        }
    }
    fclose(fp);

    retrieving_files = malloc(file_count * sizeof(FilePath_t*));
    srand(time(0));

    network = malloc(sizeof(PeerAddress_t*));
    network[0] = my_address;
    peer_count = 1;

    // Setup the client and server threads 
    pthread_t client_thread_id;
    pthread_t server_thread_id;
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {   
        pthread_create(&client_thread_id, NULL, client_thread, &peer_address);
    } 
    pthread_create(&server_thread_id, NULL, server_thread, NULL);

    // Start the threads. Note that the client is only started if a peer is 
    // provided in the config. If none is we will assume this peer is the first
    // on the network and so cannot act as a client.
    if (peer_address.ip[0] != 'x' && peer_address.port[0] != 'x')
    {
        pthread_join(client_thread_id, NULL);
    }
    pthread_join(server_thread_id, NULL);

    //If we become a client also, we replace the my_adress pointer
    //in our network[0], with the reply from the server, so we free it here.
    free(my_address);

    for (uint32_t i = 0; i < peer_count; i++)
    {
        free(network[i]);
    }
    free(network);
    free(retrieving_files);
    printf("Does it even go to here? \n");
    exit(EXIT_SUCCESS);
}