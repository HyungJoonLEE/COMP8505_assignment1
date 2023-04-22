#ifndef COMP8505_ASSIGNMENT1_CLIENT_HELPER_H
#define COMP8505_ASSIGNMENT1_CLIENT_HELPER_H

#include "common.h"


struct options_client
{
    char source_ip[40];
    char destination_ip[40];
    unsigned int src_ip;
    unsigned int dest_ip;
    in_port_t src_port;
    in_port_t dest_port;
    char file_name[20];
    int client_socket;
};


void options_init_client(struct options_client *opts);
void parse_arguments_client(int argc, char *argv[], struct options_client *opts);
void confirm_client_info(struct options_client *opts);
void options_process_client(struct options_client *opts);
uint16_t generate_random_port(void);
uint16_t calc_ip_checksum(struct iphdr *ip_header);
uint16_t calc_udp_checksum(struct udphdr *udp_header);
unsigned short encrypt_data(int ch);

#endif //COMP8505_ASSIGNMENT1_CLIENT_HELPER_H
