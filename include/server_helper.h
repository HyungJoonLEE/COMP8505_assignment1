#ifndef COMP8505_ASSIGNMENT1_SERVER_HELPER_H
#define COMP8505_ASSIGNMENT1_SERVER_HELPER_H

#include "common.h"

#define DEFAULT_DIRECTORY "."
#define BACKLOG 5


struct options_server
{
    char source_ip[40];
    char destination_ip[40];
    unsigned int src_ip;
    unsigned int dest_ip;
    in_port_t src_port;
    in_port_t dest_port;
    char file_name[20];
    int server_socket;
    int ipid;
};


int get_my_ip(struct options_server *opts);
void create_txt_file(const char* file_name);
void confirm_server_info(struct options_server *opts);
unsigned short decrypt_data(unsigned short ch);



/**
 * Initialize default option for client.
 * Memory setting for server option struct to 0 and DEFAULT port number saved.
 *
 * @param opts client struct settings
 */
void options_init_server(struct options_server *opts);


/**
 * Parse input from server IP, server port, server download directory are included.
 * Set the server option struct after parsing.
 * If there is no input, it will use DEFAULT value.
 *
 * @param argc number of argument
 * @param argv server's input
 * @param opts server option struct settings
 */
void parse_arguments_server(int argc, char *argv[], struct options_server *opts);


/**
 * Initialize network settings in server from protocol, ip, port and wait for client connection.
 *
 * @param opts client option struct settings
 */
void options_process_server(struct options_server *opts);


/**
 * Free variables that were used for client option struct setting.
 *
 * @param opts
 */
void cleanup_server(const struct options_server *opts);

void usage(char *progname);
#endif //COMP8505_ASSIGNMENT1_SERVER_HELPER_H
