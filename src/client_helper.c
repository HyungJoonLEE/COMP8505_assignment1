#include "client_helper.h"
#include "conversion.h"
#include "error.h"


void options_init_client(struct options_client *opts) {
    memset(opts, 0, sizeof(struct options_client));
}


void parse_arguments_client(int argc, char *argv[], struct options_client *opts) {
    int c;
    int option_index = 0;

    static struct option long_options[] = {
            {"source_ip", required_argument, 0, 's'},
            {"dest_ip", required_argument, 0, 'd'},
            {"source_port", required_argument, 0, 'w'},
            {"dest_port", required_argument, 0, 'e'},
            {"file", required_argument, 0, 'f'},
            {0, 0, 0, 0}
    };


    while((c = getopt_long(argc, argv, ":s:d:w:e:f:", long_options, &option_index)) != -1)   // NOLINT(concurrency-mt-unsafe)
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
                    exit(1);
                }
                break;
            }
            default:
                printf("Usage: %s [-s | --source_ip IP] [-d | --dest_ip IP] "
                       "[-w | --source_port PORT] [-e | --dest_port PORT] "
                       "[-f | --file FILENAME]\n", argv[0]);
        }
    }
}


void confirm_client_info(struct options_client *opts) {

    if (opts->src_ip == 0 && opts->dest_ip == 0) {
        puts("You need to supply a source and destination address for client mode");
        exit(1);
    }
    else {
        printf("Packet Source:        %s\n", opts->source_ip);
        printf("Packet Destination:   %s\n", opts->destination_ip);
        if(opts->src_port == 0) puts("Source Port:          random\n");
        else printf(  "Source Port:          %u\n", opts->src_port);
        if(opts->dest_port == 0) puts("Destination Port:          random\n");
        else printf("Destination Port:     %u\n", opts->dest_port);
        printf("Encoded Filename:     %s\n", opts->file_name);
        puts("[ Client Mode ]: Sending data ...");
    }
}


void options_process_client(struct options_client *opts) {
    int ch;
    struct sockaddr_in sin;
    FILE *input;


    struct send_udp {
        struct iphdr ip;
        struct udphdr udp;
    } send_udp;


    if ((input = fopen(opts->file_name, "rb")) == NULL) {
        printf("I cannot open the file %s for reading\n", opts->file_name);
        exit(1);
    }
    else
        while ((ch = fgetc(input)) != EOF) {
            sleep(1);

            /* IP header */
            send_udp.ip.ihl = 5;
            send_udp.ip.version = 4;
            send_udp.ip.tos = 0;
            send_udp.ip.id = encrypt_data(ch);
            send_udp.ip.tot_len = htons(28);    // ip header(20), udp_header(8)
            send_udp.ip.frag_off = 0;
            send_udp.ip.ttl = 64;
            send_udp.ip.protocol = IPPROTO_UDP;
            send_udp.ip.saddr = opts->src_ip;
            send_udp.ip.daddr = opts->dest_ip;

            send_udp.ip.check = calc_ip_checksum(&send_udp.ip);

            /* UDP header */
            if (opts->src_port == 0)
                send_udp.udp.uh_sport = htons(generate_random_port());
            else
                send_udp.udp.uh_sport = htons(opts->src_port);
            if (opts->dest_port == 0)
                send_udp.udp.uh_dport = htons(generate_random_port());
            else
                send_udp.udp.uh_dport = htons(opts->dest_port);
            send_udp.udp.uh_ulen = htons(8);
            send_udp.udp.uh_sum = calc_udp_checksum(&send_udp.udp);


            sin.sin_family = AF_INET;
            sin.sin_addr.s_addr = send_udp.ip.daddr;
            sin.sin_port = send_udp.udp.uh_sport;


            opts->client_socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
            if (opts->client_socket == -1) {
                printf( "socket() ERROR\n");
                close(opts->client_socket);
                exit(1);
            }


            sendto(opts->client_socket, &send_udp, 28, 0, (struct sockaddr *)&sin, sizeof(sin));
            printf("Sending Data: %c\n", ch);

            close(opts->client_socket);
        }
    fclose(input);
}


uint16_t generate_random_port(void) {
    int min_port = 1024;
    int max_port = 65535;

    srand((unsigned)time(NULL));
    return (uint16_t) ((rand() % (max_port - min_port + 1)) + min_port);
}


uint16_t calc_ip_checksum(struct iphdr *ip_header) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)ip_header;
    int count = ip_header->ihl * 4;

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


uint16_t calc_udp_checksum(struct udphdr *udp_header) {
    uint32_t sum = 0;
    uint16_t *ptr = (uint16_t *)udp_header;
    int count = udp_header->uh_ulen;

    udp_header->uh_sum = 0;

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


unsigned short encrypt_data(int ch) {
    uint16_t key = 0xABCD;
    return (unsigned short) (ch ^ key);
}

