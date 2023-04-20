#include "server_helper.h"
#include "conversion.h"
#include "error.h"



void check_root_user(int argc, char *argv[]) {
    if(geteuid() != 0) {
        printf("\nYou need to be root to run this.\n\n");
        exit(0);
    }
}


int get_src_ip(struct options_server *opts) {
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[40];

    // get the list of interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 1;
    }

    // loop through the list of interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) {
            continue;
        }

        // get the family (IPv4 or IPv6)
        family = ifa->ifa_addr->sa_family;

        // skip non-IP interfaces
        if (family != AF_INET && family != AF_INET6) {
            continue;
        }

        // get the IP address as a string
        s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) :
                                       sizeof(struct sockaddr_in6),
                        host, 40, NULL, 0, NI_NUMERICHOST);

        if (s != 0) {
            printf("getnameinfo() failed: %s\n", gai_strerror(s));
            return 1;
        }

        // print the IP address
        // printf("%s: %s\n", ifa->ifa_name, host);
        if (strncmp(host, "192.", 4) == 0) {
            strcpy(opts->source_ip, host);
        }
    }

    freeifaddrs(ifaddr); // free the linked list
}


void options_init_server(struct options_server *opts) {
    memset(opts, 0, sizeof(struct options_server));
//    get_src_ip(opts);
//    opts->src_port = SOURCE_PORT;
//    opts->dest_port = DESTINATION_PORT;
    opts->ipid = DEFAULT_IPID;
}


void parse_arguments_server(int argc, char *argv[], struct options_server *opts)
{
    int c;
    int option_index = 0;
    char ipid[5] = {0};

    static struct option long_options[] = {
            {"source_ip", required_argument, 0, 's'},
            {"dest_ip", required_argument, 0, 'd'},
            {"source_port", required_argument, 0, 'w'},
            {"dest_port", required_argument, 0, 'e'},
            {"file", required_argument, 0, 'f'},
            {"ipid", required_argument, 0, 'i'},
            {0, 0, 0, 0}
    };


    while((c = getopt_long(argc, argv, ":s:d:w:e:f:i:", long_options, &option_index)) != -1)   // NOLINT(concurrency-mt-unsafe)
    {
        switch(c)
        {
            case 's': {
                strcpy(opts->source_ip, optarg);
                opts->src_ip = host_convert(opts->source_ip);
                if (opts->src_ip == 0) strcpy(opts->source_ip, "Any Host");
                break;
            }
            case 'd': {
                opts->dest_ip = host_convert(optarg);
                if (opts->dest_ip == 0) strcpy(opts->destination_ip, "Any Host");
                else strcpy(opts->destination_ip, optarg);
                break;

            }
            case 'w': {
                opts->src_port = parse_port(optarg, 10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case 'e': {
                opts->dest_port = parse_port(optarg, 10); // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
                break;
            }
            case 'f': {
                strcpy(opts->file_name, optarg);
                if (strlen(opts->file_name) == 0) {
                    puts("You need to supply a filename [-f | --file FILENAME]");
                    exit(1);
                }
                else
                    create_txt_file(opts->file_name);
                break;
            }
            case 'i': {
                strcpy(ipid, optarg);
                opts->ipid = atoi(ipid);
                break;
            }
            default:
                printf("Usage: %s [-s | --source_ip IP] [-d | --dest_ip IP] "
                       "[-w | --source_port PORT] [-e | --dest_port PORT] "
                       "[-i | --ipid IPID] [-f | --file FILENAME]\n", argv[0]);
        }
    }
}


void confirm_server_info(struct options_server *opts) {
    if (opts->src_ip == 0 && opts->src_port == 0) {
        puts("You need to supply a source address and/or source port for server mode");
        exit(1);
    }
    // Check IP
    printf("Listening for data from IP: %s\n", opts->source_ip);

    // Check port
    if(opts->src_port == 0) printf("Listening for data bound for local port: Any Port\n");
    else printf("Listening for data bound for local port: %u\n", opts->src_port);


    printf("Decoded Filename: %s\n", opts->file_name);
    if(opts->ipid == 1) printf("Decoding Type Is: IP packet ID\n");

    puts("\n[ Server Mode ]: Listening for data");
}


void options_process_server(struct options_server *opts) {

    int ch;
    struct sockaddr_in sin;

    int option = TRUE;
    int max_sd, sd, activity, new_socket, valread;
    FILE *output;
    char buffer[1024];
    ssize_t received_bytes = 0;


    if((output = fopen(opts->file_name, "wb")) == NULL) {
        printf("Cannot open the file [ %s ] for writing\n", opts->file_name);
        exit(1);
    }

    /* read packet loop */
    while(1) {
        opts->server_socket = socket(AF_INET, SOCK_RAW, 6);
        if(opts->server_socket == -1) {
            printf( "socket() ERROR\n");
            exit(1);
        }

        read(opts->server_socket, (struct recv_udp *)&recv_pkt, 9999);
        if (opts->src_port == 0) { /* the server does not care what port we come from */

            /* IP ID header "decoding" */
            /* The ID number is converted from it's ASCII equivalent back to normal */
            if(opts->ipid==1) {
                printf("Receiving Data: %c\n",recv_pkt.ip.id);
                fprintf(output,"%c",recv_pkt.ip.id);
                fflush(output);
            }
        }

        close(opts->server_socket); /* close the socket so we don't hose the kernel */
    }

    fclose(output);
    cleanup_server(opts);
} /* end else(serverloop) function */


void cleanup_server(const struct options_server *opts) {
    close(opts->server_socket);
}


unsigned int host_convert(char *hostname)
{
    static struct in_addr i;
    struct hostent *h;
    i.s_addr = inet_addr(hostname);
    if(i.s_addr == -1)
    {
        h = gethostbyname(hostname);
        if(h == NULL)
        {
            fprintf(stderr, "cannot resolve %s\n", hostname);
            exit(0);
        }
        memcpy(h->h_name, (char *)&i.s_addr, (unsigned long) h->h_length);
    }
    return i.s_addr;
}


void create_txt_file(const char* file_name) {
    FILE *file = fopen(file_name, "wb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fclose(file);
}


