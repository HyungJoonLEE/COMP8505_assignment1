#include "common.h"


void check_root_user(int argc, char *argv[]) {
    if(geteuid() != 0) {
        printf("\nYou need to be root to run this.\n\n");
        exit(0);
    }
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
