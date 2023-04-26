#include "server_helper.h"
#include "conversion.h"
#include "error.h"


void options_init_server(struct options_server *opts) {
    memset(opts, 0, sizeof(struct options_server));
    opts->src_port = SOURCE_PORT;
}


void parse_arguments_server(int argc, char *argv[], struct options_server *opts)
{
    int c;
    int option_index = 0;

    static struct option long_options[] = {
            {"source_ip", required_argument, 0, 's'},
            {"source_port", required_argument, 0, 'w'},
            {"file", required_argument, 0, 'f'},
            {0, 0, 0, 0}
    };


    while((c = getopt_long(argc, argv, ":s:w:f:", long_options, &option_index)) != -1)   // NOLINT(concurrency-mt-unsafe)
    {
        switch(c)
        {
            case 's': {
                strcpy(opts->source_ip, optarg);
                opts->src_ip = host_convert(opts->source_ip);
                if (opts->src_ip == 0) strcpy(opts->source_ip, "Any Host");
                break;
            }
            case 'w': {
                opts->src_port = parse_port(optarg, 10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case 'f': {
                strcpy(opts->file_name, optarg);
                if (strlen(opts->file_name) == 0) {
                    puts("You need to supply a filename [-f | --file FILENAME]");
                    exit(1);
                }
                break;
            }
            default:
                printf("Usage: %s [-s | --source_ip IP] "
                       "[-w | --source_port PORT] "
                       "[-f | --file FILENAME]\n", argv[0]);
        }
    }
}


void confirm_server_info(struct options_server *opts) {
    if (opts->src_ip == 0 && opts->src_port == 0) {
        puts("You need to supply a source address and/or source port for server mode");
        exit(1);
    }
    printf("Listening for data from IP: %s\n", opts->source_ip);
    if(opts->src_port == 0) printf("Listening for data from port: Any Port\n");
    else printf("Listening for data from port: %u\n", opts->src_port);
    printf("Decoded Filename: %s\n", opts->file_name);
    puts("[ Server Mode ]: Listening for data ...");
}


void options_process_server(struct options_server *opts) {

    FILE *output;
    struct in_addr src_ip;
    char* src_ip_str;
    char* source_ip;

    struct recv_udp {
        struct iphdr ip;
        struct udphdr udp;
    } recv_pkt;


    if((output = fopen(opts->file_name, "wb")) == NULL) {
        printf("Cannot open the file [ %s ] for writing\n", opts->file_name);
        exit(1);
    }

    source_ip = malloc(sizeof(char) * 16);
    memset(source_ip, 0 ,16);
    strcpy(source_ip, opts->source_ip);

    while(1) {
        opts->server_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if(opts->server_socket == -1) {
            printf( "socket() ERROR\n");
            close(opts->server_socket);
            exit(1);
        }

        read(opts->server_socket, (struct recv_udp *)&recv_pkt, 43);
        src_ip.s_addr = recv_pkt.ip.saddr;
        src_ip_str = inet_ntoa(src_ip);
        if (opts->src_port == 0) {
            if (strcmp(src_ip_str, source_ip) == 0) {
                printf("[ %s ]: %c -> %c(Decrypted)\n", src_ip_str, recv_pkt.ip.id, decrypt_data(recv_pkt.ip.id));
                fprintf(output,"%c", decrypt_data(recv_pkt.ip.id));
                fflush(output);
            }
        } else {
            if (ntohs(recv_pkt.udp.uh_dport) == opts->src_port) {
                printf("Receiving Data: %c -> %c\n", recv_pkt.ip.id, decrypt_data(recv_pkt.ip.id));
                fprintf(output,"%c", decrypt_data(recv_pkt.ip.id));
                fflush(output);
            }
        }
        close(opts->server_socket);
    }

    fclose(output);
    close(opts->server_socket);
}


unsigned short decrypt_data(unsigned short ch) {
    uint16_t key = 0xABCD;
    return ch ^ key;
}

