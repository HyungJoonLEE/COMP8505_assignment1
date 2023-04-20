#include "client_helper.h"
#include "conversion.h"
#include "error.h"


int get_src_ip(struct options_client *opts) {
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

    freeifaddrs(ifaddr);
}


void options_init_client(struct options_client *opts) {
    memset(opts, 0, sizeof(struct options_client));
//    get_src_ip(opts);
//    opts->src_port = SOURCE_PORT;
//    opts->dest_port = DESTINATION_PORT;
    opts->ipid = DEFAULT_IPID;
}


void parse_arguments_client(int argc, char *argv[], struct options_client *opts) {
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
                strcpy(opts->destination_ip, optarg);
                opts->dest_ip = host_convert(opts->destination_ip);
                if (opts->dest_ip == 0) strcpy(opts->destination_ip, "Any Host");
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
                    cleanup_client(opts);
                    exit(1);
                }
//                else
//                    create_txt_file(opts->file_name);
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


void confirm_client_info(struct options_client *opts) {

    if (opts->src_ip == 0 && opts->dest_ip == 0) {
        puts("You need to supply a source and destination address for client mode");
        cleanup_client(opts);
        exit(1);
    }
    else {
        printf("Source Host (THIS): %s\n", opts->source_ip);
        printf("Destination Host:   %s\n", opts->destination_ip);
        if(opts->src_port == 0) puts("Source Port:    random)\n");
        else printf("Source Port:        %u\n",opts->src_port);
        printf("Destination Port:   %u\n", opts->dest_port);
        printf("Encoded Filename:   %s\n", opts->file_name);
        if(opts->ipid == 1) puts("Encoding Type   : IP ID");
        puts("[ Client Mode ]: Sending data ...");
    }
}


void options_process_client(struct options_client *opts) {
    int ch;
    struct sockaddr_in sin;
    FILE *input;


    struct send_udp {
        struct iphdr ip;
        struct udp_header udp;
    } send_udp;


    if ((input = fopen(opts->file_name, "rb")) == NULL) {
        printf("I cannot open the file %s for reading\n", opts->file_name);
        cleanup_client(opts);
        exit(1);
    }
    else
        while ((ch = fgetc(input)) != EOF) {
            sleep(1);

            /* IP header */
            send_udp.ip.ihl = 5;
            send_udp.ip.version = 4;
            send_udp.ip.tos = 0;
            send_udp.ip.tot_len = htons(28);    // ip header(20), udp_header(8)

            if (opts->ipid == 0)
                send_udp.ip.id = (int) (255.0 * rand() / (RAND_MAX + 1.0));
            else /* otherwise we "encode" it with our cheesy algorithm */
                send_udp.ip.id = encrypt_data(ch);

            send_udp.ip.frag_off = 0;
            send_udp.ip.ttl = 64;
            send_udp.ip.protocol = IPPROTO_UDP;
            send_udp.ip.saddr = opts->src_ip;
            send_udp.ip.daddr = opts->dest_ip;

            send_udp.ip.check = calc_ip_checksum(&send_udp.ip);

            /* UDP header */
            if (opts->src_port == 0)
                send_udp.udp.src_port = htons(generate_random_port());
            else
                send_udp.udp.src_port = htons(opts->src_port);
            send_udp.udp.dest_port = htons(opts->dest_port);
            send_udp.udp.length = htons(8);
            send_udp.udp.checksum = calc_udp_checksum(&send_udp.udp);


            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = send_udp.ip.daddr;
            sin.sin_port = send_udp.udp.src_port;


            opts->client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
            if (opts->client_socket == -1) {
                printf( "socket() ERROR\n");
                cleanup_client(opts);
                exit(1);
            }


            sendto(opts->client_socket, &send_udp, 28, 0, (struct sockaddr *)&sin, sizeof(sin));
            printf("Sending Data: %c\n", ch);

            close(opts->client_socket);
        }
    fclose(input);
}


void cleanup_client(const struct options_client *opts) {
    close(opts->client_socket);
}


uint16_t generate_random_port() {
    int min_port = 1024;
    int max_port = 65535;

    srand(time(NULL));
    return (uint16_t) ((rand() % (max_port - min_port + 1)) + min_port);
}


uint16_t calc_ip_checksum(struct iphdr *ip_header) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)ip_header;
    int count = ip_header->ihl * 4;

    // Initialize checksum field to 0
    ip_header->check = 0;

    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += *(uint8_t *)ptr;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}


uint16_t calc_udp_checksum(struct udp_header *udp_header) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)udp_header;
    int count = udp_header->length;

    udp_header->checksum = 0;

    // Calculate the checksum for the UDP header
    while (count > 1) {
        sum += *ptr++;
        count -= 2;
    }

    if (count > 0) {
        sum += *(uint8_t *)ptr;
    }

    // Fold the 32-bit sum into 16 bits
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return (uint16_t)~sum;
}


unsigned short encrypt_data(int ch) {
    uint16_t key = 0xABCD;
    return (unsigned short) (ch ^ key);
}

