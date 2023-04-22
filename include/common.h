#ifndef COMP8005_ASSIGNMENT1_COMMON_H
#define COMP8005_ASSIGNMENT1_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <netinet/udp.h>
#include <regex.h>
#include <openssl/aes.h>
#include <netinet/ip.h>



#define BUF_SIZE 4096
#define SOURCE_PORT 53000
#define DESTINATION_PORT 55000

#define TRUE 1
#define FALSE 0

void check_root_user(int argc, char *argv[]);
unsigned int host_convert(char *hostname);




#endif // COMP8005_ASSIGNMENT1_COMMON_H